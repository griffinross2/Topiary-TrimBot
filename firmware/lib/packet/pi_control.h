#pragma once

#include "packet.h"
#include "status.h"

#define PI_STATUS_UPDATE_INTERVAL_MS 1000

void pi_control_task();
Status pi_control_start_scanning();