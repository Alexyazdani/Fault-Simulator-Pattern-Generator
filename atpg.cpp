#include "circuit.h"
#include "levelizer.h"
#include "atpg.h"

// Truth table for AND gate
LogicVal d_and(LogicVal a, LogicVal b) {
    if (a == L_0 || b == L_0) return L_0;
    if (a == L_X || b == L_X) return L_X;
    if ((a == L_D && b == L_DB) || (a == L_DB && b == L_D)) return L_0;
    if (a == L_D || b == L_D) return L_D;
    if (a == L_DB || b == L_DB) return L_DB;
    return L_1;
}

// OR gate
LogicVal d_or(LogicVal a, LogicVal b) {
    if (a == L_1 || b == L_1) return L_1;
    if (a == L_X || b == L_X) return L_X;
    if ((a == L_D && b == L_DB) || (a == L_DB && b == L_D)) return L_1;
    if (a == L_D || b == L_D) return L_D;
    if (a == L_DB || b == L_DB) return L_DB;
    return L_0;
}

// NOT gate
LogicVal d_not(LogicVal a) {
    switch (a) {
        case L_0: return L_1;
        case L_1: return L_0;
        case L_X: return L_X;
        case L_D: return L_DB;
        case L_DB: return L_D;
        default:  return L_X;
    }
}

LogicVal d_xor(LogicVal a, LogicVal b) {
    if (a == L_X || b == L_X) return L_X;

    if (a == L_0) return b;
    if (b == L_0) return a;

    if (a == L_1) return d_not(b);
    if (b == L_1) return d_not(a);

    if (a == b) return L_0;
    return L_1;
}

 LogicVal evaluate_d_gate(NSTRUC *node) {
    LogicVal result = node->unodes[0]->d_value;
    if (node->force_d_value) {
        return node->d_value;
    }
        if (node->type == IPT || node->type == BRCH || node->type == BUFFER)
            return result;
        if (node->type == NOT)
            return d_not(result);
        for (int i = 1; i < node->fin; ++i) {
            LogicVal in_val = node->unodes[i]->d_value;
            switch (node->type) {
                case AND:  result = d_and(result, in_val); break;
                case OR:   result = d_or(result, in_val); break;
                case NAND: result = d_and(result, in_val); break;
                case NOR:  result = d_or(result, in_val); break;
                case XOR:  result = d_xor(result, in_val); break;
                case XNOR: result = d_xor(result, in_val); break;
                default:   return L_X;
            }
        }
    if ((node->type == NAND || node->type == NOR || node->type == XNOR) && result != L_X) 
        result = d_not(result);
    return result;
}

NSTRUC *fault_node = NULL;

bool inject_fault(int fault_node_num, int fault_val) {
    int index = -1;
    for (int i = 0; i < Nnodes; ++i) {
        if (Node[i].num == fault_node_num) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("Error: Fault node number %d not found.\n", fault_node_num);
        return false;
    }

    fault_node = &Node[index];

    if (fault_val == 0) {
        fault_node->d_value = L_D;   // Good = 1, Faulty = 0
        fault_node->force_d_value = true;
    } else if (fault_val == 1) {
        fault_node->d_value = L_DB;  // Good = 0, Faulty = 1
        fault_node->force_d_value = true;
    } else {
        return false;
    }

    return true;
}


void reset_d_values() {
    for (int i = 0; i < Nnodes; ++i) {
        Node[i].d_value = L_X;
        Node[i].visited = false;
        Node[i].force_d_value = false;
    }
}

void reset_non_forced() {
    for (int i = 0; i < Nnodes; ++i) {
        if (Node[i].force_d_value == false) {
            Node[i].d_value = L_X;
            Node[i].visited = false;
        }
    }
}

void reset_for_backtrack(int fault_node, int fault_val) {
    reset_d_values();
    inject_fault(fault_node, fault_val);
    for (int level = 0; level <= get_max_level(); ++level) {
        for (int i = 0; i < Nnodes; ++i) {
            if (Node[i].level == level && Node[i].fin > 0 && !Node[i].force_d_value) {
                Node[i].d_value = evaluate_d_gate(&Node[i]);
            }
        }
    }
}

const char* nodetype_to_str(int type) {
    switch (type) {
        case IPT:    return "IPT";
        case BRCH:   return "BRCH";
        case AND:    return "AND";
        case OR:     return "OR";
        case NAND:   return "NAND";
        case NOR:    return "NOR";
        case NOT:    return "NOT";
        case XOR:    return "XOR";
        case XNOR:   return "XNOR";
        case BUFFER: return "BUFFER";
        default:     return "UNKNOWN";
    }
}

BacktrackFrame backtrack_stack[MAX_STACK_DEPTH];
int stack_top = -1;

void push_state() {
    if (stack_top >= MAX_STACK_DEPTH - 1) return; // Stack overflow guard
    stack_top++;
    for (int i = 0; i < Nnodes; ++i) {
        backtrack_stack[stack_top].d_values[i] = Node[i].d_value;
        backtrack_stack[stack_top].visited[i] = Node[i].visited;
        backtrack_stack[stack_top].force_d_value[i] = Node[i].force_d_value;
    }
}
void pop_state() {
    if (stack_top < 0) return; // Stack underflow guard

    for (int i = 0; i < Nnodes; ++i) {
        Node[i].d_value = backtrack_stack[stack_top].d_values[i];
        Node[i].visited = backtrack_stack[stack_top].visited[i];
        Node[i].force_d_value = backtrack_stack[stack_top].force_d_value[i];
    }
    stack_top--;
}

DFrontierFrame dfrontier_stack[MAX_STACK_DEPTH];
int dfrontier_top = -1;
int last_popped_count = 0;

void push_dfrontier(int node_indices[], LogicVal expected_vals[], int count) {
    push_state();
    if (dfrontier_top >= MAX_STACK_DEPTH - 1) {
        printf("D-frontier stack overflow!\n");
        return;
    }
    dfrontier_top++;
    dfrontier_stack[dfrontier_top].num_nodes = count;
    for (int i = 0; i < count; i++) {
        dfrontier_stack[dfrontier_top].node_indices[i] = node_indices[i];
        dfrontier_stack[dfrontier_top].expected_vals[i] = expected_vals[i];
    }
    printf("[D-Frontier] Pushed %d nodes onto stack (stack size = %d)\n", count, dfrontier_top + 1);
}

bool pop_dfrontier(int node_indices[], LogicVal expected_vals[]) {
    pop_state();
    if (dfrontier_top < 0) {
        return false;
    }
    last_popped_count = dfrontier_stack[dfrontier_top].num_nodes;
    for (int i = 0; i < last_popped_count; i++) {
        node_indices[i] = dfrontier_stack[dfrontier_top].node_indices[i];
        expected_vals[i] = dfrontier_stack[dfrontier_top].expected_vals[i];
    }
    printf("[D-Frontier] Popped %d nodes from stack (remaining = %d)\n", last_popped_count, dfrontier_top + 1);
    dfrontier_top--;
    return true;
}

bool fault_reached_output = false;
bool tried_d_frontier[MAX_NODES] = {false};
bool inconsistent = false;
