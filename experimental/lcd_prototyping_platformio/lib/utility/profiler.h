#pragma once

#define PROFILER_ENABLED 1

void profiler_init();

void profiler_enter_function(const char* func_name);
void profiler_exit_function(const char* func_name);
void profiler_print_summary();

#if PROFILER_ENABLED
#define PROFILER_ENTER() profiler_enter_function(__func__);
#define PROFILER_EXIT() profiler_exit_function(__func__);
#else
#define PROFILER_ENTER()
#define PROFILER_EXIT()
#endif