/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

/**
 * queue.cpp - The G-code command queue
 */

#include "marlin_wrapper.h"

#include "queue.h"
GCodeQueue queue;

#include "gcode.h"

#include "module/motion.h"
#include "module/planner.h"

#if ENABLED(BINARY_FILE_TRANSFER)
  #include "../feature/binary_stream.h"
#endif

#if ENABLED(POWER_LOSS_RECOVERY)
  #include "../feature/powerloss.h"
#endif

#if ENABLED(GCODE_REPEAT_MARKERS)
  #include "../feature/repeat.h"
#endif

GCodeQueue::SerialState GCodeQueue::serial_state = { 0 };
GCodeQueue::RingBuffer GCodeQueue::ring_buffer = { 0 };

#if NO_TIMEOUTS > 0
  static millis_t last_command_time = 0;
#endif

/**
 * Track buffer underruns
 */
#if ENABLED(BUFFER_MONITORING)
  uint32_t GCodeQueue::command_buffer_underruns = 0,
           GCodeQueue::planner_buffer_underruns = 0;
  bool GCodeQueue::command_buffer_empty = false,
       GCodeQueue::planner_buffer_empty = false;
  millis_t GCodeQueue::max_command_buffer_empty_duration = 0,
           GCodeQueue::max_planner_buffer_empty_duration = 0,
           GCodeQueue::command_buffer_empty_at = 0,
           GCodeQueue::planner_buffer_empty_at = 0;

  uint8_t GCodeQueue::auto_buffer_report_interval;
  millis_t GCodeQueue::next_buffer_report_ms;
#endif

/**
 * Commit the accumulated G-code command to the ring buffer,
 * also setting its origin info.
 */
void GCodeQueue::RingBuffer::commit_command(const bool skip_ok) {
  commands[index_w].skip_ok = skip_ok;
  advance_w();
}

/**
 * Copy a command from RAM into the main command buffer.
 * Return true if the command was successfully added.
 * Return false for a full buffer, or if the 'command' is a comment.
 */
bool GCodeQueue::RingBuffer::enqueue(const char *cmd, const bool skip_ok) {
  if (*cmd == ';' || length >= BUFSIZE) return false;
  strcpy(commands[index_w].buffer, cmd);
  commit_command(skip_ok);
  return true;
}

/**
 * Enqueue with Serial Echo
 * Return true if the command was consumed
 */
bool GCodeQueue::enqueue_one(const char * const cmd) {
  //SERIAL_ECHOLNPGM("enqueue_one(\"", cmd, "\")");

  if (*cmd == 0 || ISEOL(*cmd)) return true;

  if (ring_buffer.enqueue(cmd)) {
    return true;
  }
  return false;
}

/**
 * Enqueue and return only when commands are actually enqueued.
 * Never call this from a G-code handler!
 */
void GCodeQueue::enqueue_one_now(const char * const cmd) { while (!enqueue_one(cmd)) marlin_wrapper_idle(); }

/**
 * Send an "ok" message to the host, indicating
 * that a command was successfully processed.
 *
 * With ADVANCED_OK:
 *   N<int>  Line number of the command, if any
 *   P<int>  Planner space remaining
 *   B<int>  Block queue space remaining
 */
void GCodeQueue::RingBuffer::ok_to_send() {
}

/**
 * Send a "Resend: nnn" message to the host to
 * indicate that a command needs to be re-sent.
 */
// void GCodeQueue::flush_and_request_resend(const serial_index_t serial_ind) {
// }

static bool serial_data_available() {
  return marlin_wrapper_serial_available();
}

#if NO_TIMEOUTS > 0
  // Multiserial already handles dispatch to/from multiple ports
  static bool any_serial_data_available() {
    for (uint8_t p = 0; p < NUM_SERIAL; ++p)
      if (serial_data_available(p))
        return true;
    return false;
  }
#endif

inline int read_serial() { return marlin_wrapper_serial_read(); }

#if (defined(ARDUINO_ARCH_STM32F4) || defined(ARDUINO_ARCH_STM32)) && defined(USBCON)

  /**
   * arduinoststm32's USB receive buffer is not well behaved when the buffer overflows
   *
   * This can happen when the host programs (such as Pronterface) automatically
   * send M105 temperature requests.
   */
  void GCodeQueue::flush_rx() {
    // Flush receive buffer
    for (uint8_t p = 0; p < NUM_SERIAL; ++p) {
      if (!serial_data_available(p)) continue; // No data for this port? Skip.
      while (SERIAL_IMPL.available(p)) (void)read_serial(p);
    }
  }

#endif // (ARDUINO_ARCH_STM32F4 || ARDUINO_ARCH_STM32) && USBCON

void GCodeQueue::gcode_line_error(const char * const ferr) {
  
}

#define PS_NORMAL 0
#define PS_EOL    1
#define PS_QUOTED 2
#define PS_PAREN  3
#define PS_ESC    4

inline void process_stream_char(const char c, uint8_t &sis, char (&buff)[MAX_CMD_SIZE], int &ind) {

  if (sis == PS_EOL) return;    // EOL comment or overflow

  #if ENABLED(PAREN_COMMENTS)
    else if (sis == PS_PAREN) { // Inline comment
      if (c == ')') sis = PS_NORMAL;
      return;
    }
  #endif

  else if (sis >= PS_ESC)       // End escaped char
    sis -= PS_ESC;

  else if (c == '\\') {         // Start escaped char
    sis += PS_ESC;
    if (sis == PS_ESC) return;  // Keep if quoting
  }

  #if ENABLED(GCODE_QUOTED_STRINGS)

    else if (sis == PS_QUOTED) {
      if (c == '"') sis = PS_NORMAL; // End quoted string
    }
    else if (c == '"')          // Start quoted string
      sis = PS_QUOTED;

  #endif

  else if (c == ';') {          // Start end-of-line comment
    sis = PS_EOL;
    return;
  }

  #if ENABLED(PAREN_COMMENTS)
    else if (c == '(') {        // Start inline comment
      sis = PS_PAREN;
      return;
    }
  #endif

  // Backspace erases previous characters
  if (c == 0x08) {
    if (ind) buff[--ind] = '\0';
  }
  else {
    buff[ind++] = c;
    if (ind >= MAX_CMD_SIZE - 1)
      sis = PS_EOL;             // Skip the rest on overflow
  }
}

/**
 * Handle a line being completed. For an empty line
 * keep sensor readings going and watchdog alive.
 */
inline bool process_line_done(uint8_t &sis, char (&buff)[MAX_CMD_SIZE], int &ind) {
  sis = PS_NORMAL;                    // "Normal" Serial Input State
  buff[ind] = '\0';                   // Of course, I'm a Terminator.
  const bool is_empty = (ind == 0);   // An empty line?
  if (!is_empty)
    ind = 0;                          // Start a new line
  return is_empty;                    // Inform the caller
}

/**
 * Get all commands waiting on the serial port and queue them.
 * Exit when the buffer is full or when no more characters are
 * left on the serial port.
 */
void GCodeQueue::get_serial_commands() {
  #if ENABLED(BINARY_FILE_TRANSFER)
    if (card.flag.binary_mode) {
      /**
       * For binary stream file transfer, use serial_line_buffer as the working
       * receive buffer (which limits the packet size to MAX_CMD_SIZE).
       * The receive buffer also limits the packet size for reliable transmission.
       */
      binaryStream[card.transfer_port_index.index].receive(serial_state[card.transfer_port_index.index].line_buffer);
      return;
    }
  #endif

  // If the command buffer is empty for too long,
  // send "wait" to indicate Marlin is still waiting.
  #if NO_TIMEOUTS > 0
    const millis_t ms = millis();
    if (ring_buffer.empty() && !any_serial_data_available() && ELAPSED(ms, last_command_time, NO_TIMEOUTS)) {
      SERIAL_ECHOLNPGM(STR_WAIT);
      last_command_time = ms;
    }
  #endif

  // Loop while serial characters are incoming and the queue is not full
  for (bool hadData = true; hadData;) {
    // Unless a serial port has data, this will exit on next iteration
    hadData = false;

    // Check if the queue is full and exit if it is.
    if (ring_buffer.full()) return;

    // No data for this port ? Skip it
    if (!serial_data_available()) continue;

    // Ok, we have some data to process, let's make progress here
    hadData = true;

    const int c = read_serial();
    if (c < 0) {
      continue;
    }

    const char serial_char = (char)c;
    SerialState &serial = serial_state;

    if (ISEOL(serial_char)) {

      // Reset our state, continue if the line was empty
      if (process_line_done(serial.input_state, serial.line_buffer, serial.count))
        continue;

      char* command = serial.line_buffer;

      while (*command == ' ') command++;                   // Skip leading spaces
      char *npos = (*command == 'N') ? command : nullptr;  // Require the N parameter to start the line

      if (npos) {

        const bool M110 = !!strstr(command, "M110");

        if (M110) {
          char* n2pos = strchr(command + 4, 'N');
          if (n2pos) npos = n2pos;
        }

        const long gcode_N = strtol(npos + 1, nullptr, 10);

        // The line number must be in the correct sequence.
        if (gcode_N != serial.last_N + 1 && !M110) {
          // A request-for-resend line was already in transit so we got two - oops!
          if (WITHIN(gcode_N, serial.last_N - 1, serial.last_N)) continue;
          // A corrupted line or too high, indicating a lost line
          gcode_line_error(STR_ERR_LINE_NO);
          break;
        }

        char *apos = strrchr(command, '*');
        if (apos) {
          uint8_t checksum = 0, count = uint8_t(apos - command);
          while (count) checksum ^= command[--count];
          if (strtol(apos + 1, nullptr, 10) != checksum) {
            gcode_line_error(STR_ERR_CHECKSUM_MISMATCH);
            break;
          }
        }
        else {
          gcode_line_error(STR_ERR_NO_CHECKSUM);
          break;
        }

        serial.last_N = gcode_N;
      }
      #if HAS_MEDIA
        // Pronterface "M29" and "M29 " has no line number
        else if (card.flag.saving && !is_M29(command)) {
          gcode_line_error(F(STR_ERR_NO_CHECKSUM), p);
          break;
        }
      #endif

      //
      // Movement commands give an alert when the machine is stopped
      //

      // Process critical commands early
      if (command[0] == 'M') switch (command[3]) {
        case '2': if (command[2] == '1' && command[1] == '1') marlin_wrapper_kill(); break;
        case '0': if (command[1] == '4' && command[2] == '1') quickstop_stepper(); break;
      }

      #if NO_TIMEOUTS > 0
        last_command_time = ms;
      #endif

      // Add the command to the queue
      ring_buffer.enqueue(serial.line_buffer, false OPTARG(HAS_MULTI_SERIAL, p));
    }
    else
      process_stream_char(serial_char, serial.input_state, serial.line_buffer, serial.count);

  } // queue has space, serial has data
}

#if HAS_MEDIA

  /**
   * Get lines from the SD Card until the command buffer is full
   * or until the end of the file is reached. Because this method
   * always receives complete command-lines, they can go directly
   * into the main command queue.
   */
  inline void GCodeQueue::get_sdcard_commands() {
    static uint8_t sd_input_state = PS_NORMAL;

    // Get commands if there are more in the file
    if (!card.isStillFetching()) return;

    int sd_count = 0;
    while (!ring_buffer.full() && !card.eof()) {
      const int16_t n = card.get();
      const bool card_eof = card.eof();
      if (n < 0 && !card_eof) { SERIAL_ERROR_MSG(STR_SD_ERR_READ); continue; }

      CommandLine &command = ring_buffer.commands[ring_buffer.index_w];
      const char sd_char = (char)n;
      const bool is_eol = ISEOL(sd_char);
      if (is_eol || card_eof) {

        // Reset stream state, terminate the buffer, and commit a non-empty command
        if (!is_eol && sd_count) ++sd_count;          // End of file with no newline
        if (!process_line_done(sd_input_state, command.buffer, sd_count)) {

          // M808 L saves the sdpos of the next line. M808 loops to a new sdpos.
          TERN_(GCODE_REPEAT_MARKERS, repeat.early_parse_M808(command.buffer));

          #if DISABLED(PARK_HEAD_ON_PAUSE)
            // When M25 is non-blocking it can still suspend SD commands
            // Otherwise the M125 handler needs to know SD printing is active
            if (command.buffer[0] == 'M' && command.buffer[1] == '2' && command.buffer[2] == '5' && !NUMERIC(command.buffer[3]))
              card.pauseSDPrint();
          #endif

          // Put the new command into the buffer (no "ok" sent)
          ring_buffer.commit_command(true);

          // Prime Power-Loss Recovery for the NEXT commit_command
          TERN_(POWER_LOSS_RECOVERY, recovery.cmd_sdpos = card.getIndex());
        }

        if (card.eof()) card.fileHasFinished();         // Handle end of file reached
      }
      else
        process_stream_char(sd_char, sd_input_state, command.buffer, sd_count);
    }
  }

#endif // HAS_MEDIA

/**
 * Add to the circular command queue the next command from:
 *  - The command-injection queues (injected_commands_P, injected_commands)
 *  - The active serial input (usually USB)
 *  - The SD card file being actively printed
 */
void GCodeQueue::get_available_commands() {
  if (ring_buffer.full()) return;

  get_serial_commands();

  TERN_(HAS_MEDIA, get_sdcard_commands());
}

/**
 * Run the entire queue in-place. Blocks SD completion/abort until complete.
 */
void GCodeQueue::exhaust() {
  while (ring_buffer.occupied()) advance();
  planner.synchronize();
}

/**
 * Get the next command in the queue, optionally log it to SD, then dispatch it
 */
void GCodeQueue::advance() {
  // Return if the G-code buffer is empty
  if (ring_buffer.empty()) {
    #if ENABLED(BUFFER_MONITORING)
      if (!command_buffer_empty) {
        command_buffer_empty = true;
        command_buffer_underruns++;
        command_buffer_empty_at = millis();
      }
    #endif
    return;
  }

  #if ENABLED(BUFFER_MONITORING)
    if (command_buffer_empty) {
      command_buffer_empty = false;
      const millis_t command_buffer_empty_duration = millis() - command_buffer_empty_at;
      NOLESS(max_command_buffer_empty_duration, command_buffer_empty_duration);
    }
  #endif



  #if HAS_MEDIA

    if (card.flag.saving) {
      char * const cmd = ring_buffer.peek_next_command_string();
      if (is_M29(cmd)) {
        // M29 closes the file
        card.closefile();
        SERIAL_ECHOLNPGM(STR_FILE_SAVED);

        #if !defined(__AVR__) || !defined(USBCON)
          #if ENABLED(SERIAL_STATS_DROPPED_RX)
            SERIAL_ECHOLNPGM("Dropped bytes: ", MYSERIAL1.dropped());
          #endif
          #if ENABLED(SERIAL_STATS_MAX_RX_QUEUED)
            SERIAL_ECHOLNPGM("Max RX Queue Size: ", MYSERIAL1.rxMaxEnqueued());
          #endif
        #endif

        ok_to_send();
      }
      else {
        // Write the string from the read buffer to SD
        card.write_command(cmd);
        if (card.flag.logging)
          gcode.process_next_command(); // The card is saving because it's logging
        else
          ok_to_send();
      }
    }
    else
      gcode.process_next_command();

  #else

    gcode.process_next_command();

  #endif // HAS_MEDIA

  // The queue may be reset by a command handler or by code invoked by idle() within a handler
  ring_buffer.advance_r();
}

#if ENABLED(BUFFER_MONITORING)

  void GCodeQueue::report_buffer_statistics() {
    SERIAL_ECHOLNPGM("D576"
      " P:", planner.moves_free(),         " ", planner_buffer_underruns, " (", max_planner_buffer_empty_duration, ")"
      " B:", BUFSIZE - ring_buffer.length, " ", command_buffer_underruns, " (", max_command_buffer_empty_duration, ")"
    );
    command_buffer_underruns = planner_buffer_underruns = 0;
    max_command_buffer_empty_duration = max_planner_buffer_empty_duration = 0;
  }

  void GCodeQueue::auto_report_buffer_statistics() {
    // Bit of a hack to try to catch planner buffer underruns without having logic
    // running inside Stepper::block_phase_isr
    const millis_t ms = millis();
    if (planner.movesplanned() == 0) {
      if (!planner_buffer_empty) { // the planner buffer wasn't empty, but now it is
        planner_buffer_empty = true;
        planner_buffer_underruns++;
        planner_buffer_empty_at = ms;
      }
    }
    else if (planner_buffer_empty) { // the planner buffer was empty, but now it's not
      planner_buffer_empty = false;
      const millis_t planner_buffer_empty_duration = ms - planner_buffer_empty_at;
      NOLESS(max_planner_buffer_empty_duration, planner_buffer_empty_duration); // if it's longer than the currently tracked max duration, replace it
    }

    if (auto_buffer_report_interval && ELAPSED(ms, next_buffer_report_ms)) {
      next_buffer_report_ms = ms + 1000UL * auto_buffer_report_interval;
      PORT_REDIRECT(SerialMask::All);
      report_buffer_statistics();
      PORT_RESTORE();
    }
  }

#endif // BUFFER_MONITORING
