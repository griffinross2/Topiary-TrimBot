#include "file_sender.h"

#include "packet.h"
#include "filesystem.h"

#include "string.h"

static FileSenderStatus s_file_sender_status = FILE_SENDER_STATUS_IDLE;
static char s_current_filename[64];
static uint8_t s_file_data_buf[MAX_PACKET_DATA_SIZE];
static size_t s_file_data_buf_len = 0;
static bool s_waiting_for_ack = false;
static bool s_last_chunk = false;
static uint32_t s_chunk_idx = 0;
static uint32_t s_resend_chunk_idx = 0;

void file_sender_task() {
    // Big state machine
    switch (s_file_sender_status) {
        case FILE_SENDER_STATUS_IDLE:
            break;

        case FILE_SENDER_STATUS_START: {
            // Send the start packet and once it goes through, just sit and wait
            // for ack
            if (!s_waiting_for_ack) {
                int res = packet_send((uint8_t*)s_current_filename,
                                      strlen(s_current_filename),
                                      PACKET_TYPE_FILE_START);

                if (res >= 0) {
                    // Successfully sent, now wait for ack
                    s_waiting_for_ack = true;
                }
            }

            break;
        }

        case FILE_SENDER_STATUS_SENDING: {
            // Send the next chunk, and once it goes through, move to the next
            // chunk

            int res = packet_send(s_file_data_buf, s_file_data_buf_len,
                                  PACKET_TYPE_FILE_CHUNK);

            if (res >= 0) {
                TRACE_PRINTF("Sent chunk idx: %d, actual len: %d\n",
                             s_chunk_idx, s_file_data_buf_len);

                if (s_last_chunk) {
                    // This was the last chunk, move to end
                    s_file_sender_status = FILE_SENDER_STATUS_END;
                    s_last_chunk = false;
                    break;
                }

                size_t actual_len;
                if (filesystem_read_file(
                        (char*)s_file_data_buf + sizeof(s_chunk_idx),
                        MAX_PACKET_DATA_SIZE - sizeof(s_chunk_idx),
                        &actual_len) == STATUS_OK) {
                    actual_len += sizeof(s_chunk_idx);
                    s_file_data_buf_len = actual_len;
                } else {
                    s_file_sender_status = FILE_SENDER_STATUS_ERROR;
                }

                // Prepend the chunk idx to the data
                memcpy(s_file_data_buf, &s_chunk_idx, sizeof(s_chunk_idx));
                s_chunk_idx++;

                if (actual_len < MAX_PACKET_DATA_SIZE) {
                    // This was the last chunk, move to end after sending
                    s_last_chunk = true;
                }
            }

            break;
        }

        case FILE_SENDER_STATUS_RESENDING:
            break;

        case FILE_SENDER_STATUS_END: {
            // Send the end packet and once it goes through, just sit and wait
            // for ack
            if (!s_waiting_for_ack) {
                int res = packet_send(nullptr, 0, PACKET_TYPE_FILE_END);

                if (res >= 0) {
                    // Successfully sent, now wait for ack
                    s_waiting_for_ack = true;
                }
            }

            break;
        }

        default:
            break;
    }
}

void file_sender_give_packet(PacketID id, const uint8_t* data,
                             int data_length) {
    switch (id.type) {
        case ((PacketID)PACKET_TYPE_FILE_START).type:
            if (id.ack && s_file_sender_status == FILE_SENDER_STATUS_START) {
                // If we were waiting for an ack to the start, now go to
                // sending
                s_file_sender_status = FILE_SENDER_STATUS_SENDING;
                s_last_chunk = false;
                s_waiting_for_ack = false;

                // Preload the first chunk of data
                s_chunk_idx = 0;
                size_t actual_len;
                if (filesystem_read_file(
                        (char*)s_file_data_buf + sizeof(s_chunk_idx),
                        MAX_PACKET_DATA_SIZE - sizeof(s_chunk_idx),
                        &actual_len) == STATUS_OK) {
                    actual_len += sizeof(s_chunk_idx);
                    s_file_data_buf_len = actual_len;
                } else {
                    TRACE_PRINTF("Failed to read file data!\n");
                    s_file_sender_status = FILE_SENDER_STATUS_ERROR;
                }

                // Prepend the chunk idx to the data
                memcpy(s_file_data_buf, &s_chunk_idx, sizeof(s_chunk_idx));
                s_chunk_idx++;

                if (actual_len < MAX_PACKET_DATA_SIZE) {
                    // This was the last chunk, move to end after sending
                    s_last_chunk = true;
                }

            } else {
                s_file_sender_status = FILE_SENDER_STATUS_ERROR;
            }
            break;

        case ((PacketID)PACKET_TYPE_FILE_CHUNK).type:
            // Should never receive this
            s_file_sender_status = FILE_SENDER_STATUS_ERROR;
            break;

        case ((PacketID)PACKET_TYPE_FILE_RESEND).type:
            if (!id.ack && s_file_sender_status == FILE_SENDER_STATUS_END) {
                // If we were waiting for an ack to the end, but get
                // resend, go to resending
                s_file_sender_status = FILE_SENDER_STATUS_RESENDING;
            } else {
                s_file_sender_status = FILE_SENDER_STATUS_ERROR;
            }
            break;

        case ((PacketID)PACKET_TYPE_FILE_END).type:
            if (id.ack && s_file_sender_status == FILE_SENDER_STATUS_END) {
                // If we were waiting for an ack to the end, now go to
                // idle
                s_file_sender_status = FILE_SENDER_STATUS_IDLE;
                s_waiting_for_ack = false;

                // Close the file
                if (filesystem_close_file() != STATUS_OK) {
                    TRACE_PRINTF("Failed to close file after sending!\n");
                }
            } else {
                s_file_sender_status = FILE_SENDER_STATUS_ERROR;
            }
            break;

        default:
            break;
    }
}

Status file_sender_send_file(const char* filename) {
    if (s_file_sender_status != FILE_SENDER_STATUS_IDLE &&
        s_file_sender_status != FILE_SENDER_STATUS_ERROR) {
        TRACE_PRINTF("File sender is busy sending another file!\n");

        return STATUS_BUSY;
    }

    if (filesystem_open_file(filename) != STATUS_OK) {
        TRACE_PRINTF("Failed to open file to send: %s\n", filename);

        return STATUS_ERROR;
    }

    snprintf((char*)s_current_filename, sizeof(s_current_filename), "%s",
             filename);

    s_file_sender_status = FILE_SENDER_STATUS_START;

    return STATUS_OK;
}