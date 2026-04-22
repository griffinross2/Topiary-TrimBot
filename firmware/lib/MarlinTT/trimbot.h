#pragma once

#include "types.h"
#include "macros.h"

extern float segments_per_second;

float constexpr D = DIST_TT_BASE, TY = DIST_REV_Y, TZ = DIST_REV_Z,
                H = DIST_TT_GANTRY;  // Float constants for TrimBot calculations

void forward_kinematics(const float theta_c, const float y, const float z,
                        const float theta_b);
void home_TRIMBOT();
void inverse_kinematics(const xyz_pos_t& raw);
void trimbot_set_axis_is_at_home(const AxisEnum axis);
void trimbot_report_positions();