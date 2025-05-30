#ifndef ATPG_H
#define ATPG_H

#define MAX_NODES 10000
#define MAX_INPUTS 1000
#define MAX_FANIN 32
#define MAX_STACK_DEPTH 10000
// #define MAX_DFRONTIER_DEPTH 100

typedef struct {
    LogicVal d_values[MAX_NODES];
    bool visited[MAX_NODES];
    bool force_d_value[MAX_NODES];
} BacktrackFrame;

extern BacktrackFrame backtrack_stack[MAX_STACK_DEPTH];
extern int stack_top;

void push_state();
void pop_state();

typedef struct {
    int node_indices[MAX_INPUTS];
    LogicVal expected_vals[MAX_INPUTS];
    int num_nodes;
} DFrontierFrame;

extern DFrontierFrame dfrontier_stack[MAX_STACK_DEPTH];
extern int dfrontier_top;
extern int last_popped_count;

void push_dfrontier(int node_indices[], LogicVal expected_vals[], int count);
bool pop_dfrontier(int node_indices[], LogicVal expected_vals[]);

extern bool fault_reached_output;
extern bool tried_d_frontier[MAX_NODES];
extern bool inconsistent;

void reset_non_forced();

#endif
