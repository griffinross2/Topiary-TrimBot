#include "MarlinConfig.h"

#if ENABLED(TRIMBOT)

#include "trimbot.h"
#include "motion.h"

// For homing:
#include "planner.h"
#include "endstops.h"
// #include "../lcd/marlinui.h"

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
}

// inverse kinematic model
void inverse_kinematics(const xyz_pos_t& raw) {
    const float theta_b = current_position[E_AXIS];  // MAPS E-AXIS TO THETA_B
    const float y = SQRT(POW(raw.x, 2) + POW((raw.y - D), 2)) + D -
                    TY * cos(RADIANS(90 - theta_b)) +
                    TZ * sin(RADIANS(90 - theta_b));
    const float z = H - raw.z - TY * sin(RADIANS(90 - theta_b)) -
                    TZ * cos(RADIANS(90 - theta_b));
    const float theta_c = ATAN2(raw.x, (raw.y - D));

    delta.set(DEGREES(theta_c), y, z, DEGREES(theta_b));
}

// homing function
void home_TRIMBOT() {}

// set axis at home function
void trimbot_set_axis_is_at_home(const AxisEnum axis) {}

// report positions function
void trimbot_report_positions() {}

#endif  // TRIMBOT