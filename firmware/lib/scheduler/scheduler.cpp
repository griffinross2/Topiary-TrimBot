#include "scheduler.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    char name[SCHEDULER_MAX_TASK_NAME_LEN];  // Task name
    std::function<void()> task_func;         // Task function
    uint32_t interval_ticks;                 // Task interval in ticks
    uint32_t priority;       // Task priority (lower number = higher priority)
    uint32_t last_run_tick;  // Last tick when the task was run
    uint32_t ticks_in_task;  // Total ticks spent in this task
} SchedulerTask;

static SchedulerTask s_tasks[SCHEDULER_MAX_TASKS];
static uint32_t s_task_count = 0;
static uint32_t s_idle_ticks = 0;
static uint32_t s_idle_start_tick = 0;
static bool s_idle = true;

Status scheduler_init() {
    s_task_count = 0;
    s_idle_ticks = 0;
    s_idle_start_tick = get_tick_ms();

    return STATUS_OK;
}

Status scheduler_add_task(const char* task_name,
                          std::function<void()> task_func,
                          uint32_t interval_ticks, uint32_t priority) {
    // Create and add the task`
    SchedulerTask task = {.name = {0},
                          .task_func = task_func,
                          .interval_ticks = interval_ticks,
                          .priority = priority,
                          .last_run_tick = get_tick_ms()};

    strncpy(task.name, task_name, SCHEDULER_MAX_TASK_NAME_LEN - 1);
    task.name[SCHEDULER_MAX_TASK_NAME_LEN - 1] = '\0';

    // Add the task to the scheduler
    if (s_task_count < SCHEDULER_MAX_TASKS) {
        s_tasks[s_task_count++] = task;
    } else {
        // Too many tasks!
        return STATUS_ERROR;
    }
    return STATUS_OK;
}

void scheduler_run() {
    uint32_t current_tick = get_tick_ms();

    // Run tasks:
    // Higher priority (lower number) tasks will always run before those
    // that are lower priority. This means that the lower priority tasks
    // will run only if the higher priority ones are blocked.
    // Tasks of equal priority will not run in round-robin, but
    // will have priority in the order that they were added.

    SchedulerTask* next_task = nullptr;

    // Find the next task to run
    for (uint32_t i = 0; i < s_task_count; i++) {
        uint32_t next_run_tick =
            s_tasks[i].last_run_tick + s_tasks[i].interval_ticks;
        bool blocked = next_run_tick > current_tick;

        if (!blocked &&
            (!next_task || s_tasks[i].priority < next_task->priority)) {
            // If the task is ready and has higher priority, select it
            next_task = &s_tasks[i];
        }
    }

    // If no task was run, return
    if (!next_task) {
        // Track idle ticks
        s_idle_start_tick = get_tick_ms();
        s_idle = true;
        return;
    }

    // No longer idle
    if (s_idle) {
        // Track idle ticks
        s_idle = false;
        s_idle_ticks += (get_tick_ms() - s_idle_start_tick);
    }

    // Run the selected task
    uint32_t task_start_tick = get_tick_ms();
    if (next_task->task_func) {
        next_task->last_run_tick = current_tick;
        next_task->task_func();
    }
    uint32_t task_end_tick = get_tick_ms();

    // Update the total ticks spent in this task
    next_task->ticks_in_task += task_end_tick - task_start_tick;
}

void scheduler_print_summary() {
    for (uint32_t i = 0; i < s_task_count; i++) {
        printf("%-*s : %lu\n", SCHEDULER_MAX_TASK_NAME_LEN - 1, s_tasks[i].name,
               s_tasks[i].ticks_in_task);
    }
    printf("%-*s : %lu\n", SCHEDULER_MAX_TASK_NAME_LEN - 1, "idle",
           s_idle_ticks);
}