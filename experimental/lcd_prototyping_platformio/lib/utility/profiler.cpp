#include "profiler.h"

#include "timing.h"
#include "status.h"

#include <unordered_map>
#include <vector>

typedef struct ProfilerTreeNode {
    std::string func_name;
    long long unsigned call_time_ms;
    long long unsigned time_elapsed_ms;
    ProfilerTreeNode* parent;
    std::vector<ProfilerTreeNode*> children;
} ProfilerTreeNode;

ProfilerTreeNode g_profiler_root = {"global", 0, 0, nullptr, {}};
ProfilerTreeNode* g_profiler_current_node = &g_profiler_root;

void profiler_init() {
    g_profiler_root.call_time_ms = get_tick_ms();
}

void profiler_enter_function(const char* func_name) {
    // First check if this function is already a child
    for (ProfilerTreeNode* child : g_profiler_current_node->children) {
        if (child->func_name == func_name) {
            g_profiler_current_node = child;

            // Start timing
            g_profiler_current_node->call_time_ms = get_tick_ms();
            return;
        }
    }

    // If not, create a new child node
    ProfilerTreeNode* new_node = new ProfilerTreeNode{
        func_name, get_tick_ms(), 0, g_profiler_current_node, {}};
    g_profiler_current_node->children.push_back(new_node);
    g_profiler_current_node = new_node;
}

void profiler_exit_function(const char* func_name) {
    // If the function name doesn't match the current node, something went wrong
    if (g_profiler_current_node->func_name != func_name) {
        TRACE_PRINTF("Profiler error: exiting function %s but should be %s\n",
                     func_name, g_profiler_current_node->func_name.c_str());
        while (1) {
        }
    }

    // Calculate elapsed time
    long long exit_time_ms = get_tick_ms();
    g_profiler_current_node->time_elapsed_ms +=
        exit_time_ms - g_profiler_current_node->call_time_ms;

    // Move back up to parent
    if (g_profiler_current_node->parent == nullptr) {
        TRACE_PRINTF(
            "Profiler error: trying to exit function %s but already at root\n",
            func_name);
        while (1) {
        }
    }
    g_profiler_current_node = g_profiler_current_node->parent;
}

void profiler_print_recursive(ProfilerTreeNode* node, int depth,
                              long long unsigned parent_time_ms) {
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    size_t string_width = (40 - depth * 2) > 0 ? (40 - depth * 2) : 20;
    printf("%-*s: %9lu ms - %3u%%\n", string_width, node->func_name.c_str(),
           (long unsigned)node->time_elapsed_ms,
           parent_time_ms > 0
               ? (unsigned)(node->time_elapsed_ms * 100ULL / parent_time_ms)
               : 0);
    for (ProfilerTreeNode* child : node->children) {
        profiler_print_recursive(child, depth + 1, node->time_elapsed_ms);
    }
}

void profiler_print_summary() {
    printf("Profiler Summary:\n\n");

    // Traverse the tree and print out the time spent in each function
    long long unsigned root_elapsed =
        get_tick_ms() - g_profiler_root.call_time_ms;
    g_profiler_root.time_elapsed_ms = root_elapsed;
    profiler_print_recursive(&g_profiler_root, 0, root_elapsed);
    printf("\n");
}