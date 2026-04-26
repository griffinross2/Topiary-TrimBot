#pragma once

// A simple cooperative scheduler to organize the main event loop.

#include "status.h"
#include "timing.h"

#include <functional>

#define SCHEDULER_MAX_TASKS 16
#define SCHEDULER_MAX_TASK_NAME_LEN 32

// Initialize the scheduler
Status scheduler_init();

// Add a task to the scheduler with a specified interval and priority
Status scheduler_add_task(const char* task_name,
                          std::function<void()> task_func,
                          uint32_t interval_ticks, uint32_t priority);

// Run the scheduler loop
void scheduler_run();

// Print a summary of task times
void scheduler_print_summary();