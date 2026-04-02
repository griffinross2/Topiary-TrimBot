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

#include "../../MarlinConfig.h"

#if HAS_HOME_OFFSET

#include "../gcode.h"
#include "../../module/motion.h"

/**
 * M206: Set Additional Homing Offset (X Y Z). SCARA aliases T=X, P=Y
 *
 * *** TODO: Deprecate M206 for SCARA in favor of M665.
 */
void GcodeSuite::M206() {
  if (!parser.seen_any()) return M206_report();
  LOOP_NUM_AXES(a)
    if (parser.seenval(AXIS_CHAR(a))) set_home_offset((AxisEnum)a, parser.value_axis_units((AxisEnum)a));
  #if ENABLED(MORGAN_SCARA)
    if (parser.seenval('T')) set_home_offset(A_AXIS, parser.value_float()); // Theta
    if (parser.seenval('P')) set_home_offset(B_AXIS, parser.value_float()); // Psi
  #endif

  report_current_position();
}

void GcodeSuite::M206_report(const bool forReplay/*=true*/) {
  TERN_(MARLIN_SMALL_BUILD, return);
}

/**
 * M428: Set home_offset based on the distance between the
 *       current_position and the nearest "reference point."
 *       If an axis is past center its endstop position
 *       is the reference-point. Otherwise it uses 0. This allows
 *       the Z offset to be set near the bed when using a max endstop.
 *
 *       M428 can't be used more than 2cm away from 0 or an endstop.
 *
 *       Use M206 to set these values directly.
 */
void GcodeSuite::M428() {
  if (homing_needed_error()) return;

  xyz_float_t diff;
  LOOP_NUM_AXES(i) {
    diff[i] = base_home_pos((AxisEnum)i) - current_position[i];
    if (!WITHIN(diff[i], -20, 20) && home_dir((AxisEnum)i) > 0)
      diff[i] = -current_position[i];
    if (!WITHIN(diff[i], -20, 20)) {
      return;
    }
  }

  LOOP_NUM_AXES(i) set_home_offset((AxisEnum)i, diff[i]);
  report_current_position();
}

#endif // HAS_HOME_OFFSET
