#include "MarlinConfig.h"

#if ENABLED(TRIMBOT)

#include "trimbot.h"
#include "module/motion.h"

// For homing:
#include "module/planner.h"
#include "module/endstops.h"
// #include "../lcd/marlinui.h"

#include <stdio.h>

float segments_per_second;

// forward kinematic model
void forward_kinematics(const float theta_c, const float y, const float z,
                        const float theta_b) {  // cyzb -> xyze
    cartes.x = D * sin(RADIANS(theta_c)) -
               sin(RADIANS(theta_c)) * (y + TY * cos(RADIANS(90 - theta_b)) -
                                        TZ * sin(RADIANS(90 - theta_b)));
    cartes.y = -D * cos(RADIANS(theta_c)) +
               cos(RADIANS(theta_c)) * (y + TY * cos(RADIANS(90 - theta_b)) -
                                        TZ * sin(RADIANS(90 - theta_b))) +
               D;
    cartes.z = H - z + TY * sin(RADIANS(90 - theta_b)) +
               TZ * cos(RADIANS(90 - theta_b));

    // printf(
    //     "Forward kinematics: theta_c=%.2f, y=%.2f, z=%.2f, theta_b=%.2f -> "
    //     "x=%.2f, y=%.2f, z=%.2f\n",
    //     theta_c, y, z, theta_b, cartes.x, cartes.y, cartes.z);
}

// inverse kinematic model
void inverse_kinematics(const xyz_pos_t& raw) {
    const float theta_b = raw.i;
    const float y = D - SQRT(POW(raw.x, 2) + POW(raw.y, 2)) -
                    TY * cos(RADIANS(90 - theta_b)) -
                    TZ * sin(RADIANS(90 - theta_b));
    const float z = H - raw.z + TY * sin(RADIANS(90 - theta_b)) +
                    TZ * cos(RADIANS(90 - theta_b));
    const float theta_c = ATAN2(raw.x, -1 * raw.y);

    delta.set(0, y, z, theta_b, DEGREES(theta_c));

    // printf(
    //     "Inverse kinematics: x=%.2f, y=%.2f, z=%.2f, cutter_angle=%.2f -> "
    //     "turntable_angle=%.2f, cutter_angle=%.2f, y=%.2f, z=%.2f\n",
    //     raw.x, raw.y, raw.z, raw.i, theta_c, theta_b, y, z);
}

// homing function
void home_TRIMBOT() {}

// set axis at home function
void trimbot_set_axis_is_at_home(const AxisEnum axis) {}

// report positions function
void trimbot_report_positions() {}

#endif  // TRIMBOT