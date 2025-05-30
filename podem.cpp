#include "podem.h"
#include "levelizer.h"
#include "scoap.h"
#include <unordered_set>
#include <cassert>
#include <queue>
#include <vector>
#include <climits>
#include <algorithm>

extern std::string dfrontier_mode;
extern std::string jfrontier_mode;

struct pair_hash {
    size_t operator()(const std::pair<int, LogicVal>& p) const {
        return std::hash<int>()(p.first) ^ std::hash<int>()(static_cast<int>(p.second) << 1);
    }
};


#define MAX_PATH_LEN 10000
#define MAX_DECISIONS 10000

static long long podem_rec_calls = 0;
static long long backtrace_calls = 0;

// READ ../auto-tests-phase3/ckts/c3.ckt
// PODEM 1 0 ../auto-tests-phase3/outputs/dalg/c3_dalg_1_0.out
// QUIT
NSTRUC* current_path_nodes[MAX_PATH_LEN];
LogicVal current_path_values[MAX_PATH_LEN];
int current_path_len = 0;
// std::queue<NSTRUC*> event_queue;

struct LevelCmp {
    bool operator()(NSTRUC* a, NSTRUC* b) const {
        return a->level > b->level; // min-heap by level
    }
};
std::priority_queue<NSTRUC*, std::vector<NSTRUC*>, LevelCmp> event_queue;
std::vector<bool> enqueued(Nnodes, false);

void enqueue(NSTRUC* node) {
    assert(node != nullptr);
    assert(node->index >= 0 && node->index < (int)enqueued.size());
    if (!enqueued[node->index]) {
     //printf("Enqueuing node %d for forward implication\n", node->num);
        enqueued[node->index] = true;
        event_queue.push(node);
    }
}
typedef struct {
    NSTRUC* nodes[MAX_PATH_LEN];
    LogicVal values[MAX_PATH_LEN];
    int length;
} DecisionPath;

DecisionPath decision_stack[MAX_DECISIONS];
int decision_stack_len = 0;

// DecisionPath top = decision_stack[--decision_stack_len];

void push_decision_path(NSTRUC* path_nodes[], LogicVal path_values[], int path_len) {
    if (decision_stack_len >= MAX_DECISIONS) {
     //printf("Decision stack overflow!\n");
        return;
    }
    DecisionPath* dp = &decision_stack[decision_stack_len++];
    dp->length = path_len;
    for (int i = 0; i < path_len; ++i) {
        dp->nodes[i] = path_nodes[i];
        dp->values[i] = path_values[i];
    }
}

DecisionPath pop_decision_path() {
    if (decision_stack_len == 0) {
     //printf("Decision stack underflow!\n");
        // DecisionPath empty = { .length = 0 };
        DecisionPath empty = {};
        empty.length = 0;
        return empty;
    }
    return decision_stack[--decision_stack_len];
}

LogicVal invert_logicval(LogicVal val) {
    switch (val) {
        case L_0: return L_1;
        case L_1: return L_0;
        default:  return val;
    }
}

bool is_primary_output(NSTRUC* node) {
    for (int i = 0; i < Npo; ++i) {
        if (Poutput[i] == node) return true;
    }
    return false;
}

bool x_path_check(NSTRUC* node) {
    if (!node) return false;
    if (is_primary_output(node)) return true;
    for (int i = 0; i < node->fout; ++i) {
        NSTRUC* fanout = node->dnodes[i];
        if (!fanout) continue; 
        if (fanout->d_value == L_X || fanout->d_value == L_D || fanout->d_value == L_DB) {
            if (x_path_check(fanout)) return true;
        }
    }
    return false;
}


// void flip_decision_stack() {
//     for (int i = 0; i < top.length; ++i) {
//       //printf("    Flipping decision stack node %d from %s to %s\n", top.nodes[i]->num, logicval_to_str(top.values[i]), logicval_to_str(invert_logicval(top.values[i])));
//         NSTRUC* n = top.nodes[i];
//         LogicVal v = top.values[i];
//         n->d_value = invert_logicval(v);
//         enqueue(n);
//         n->visited = false;
//         // if (n->type == IPT) {
//         //     n->force_d_value = true;
//         // } else {
//         //     n->force_d_value = false;
//         //     n->visited = false;
//         //     // n->d_value = L_X;
//         // }
//     }
// }
void flip_decision_stack(const DecisionPath& top) {
    for (int i = 0; i < top.length; ++i) {
        NSTRUC* n = top.nodes[i];
        LogicVal v = top.values[i];
        n->d_value = invert_logicval(v);
        enqueue(n);
        n->visited = false;
    }
}


// void reset_decision_stack() {
void reset_decision_stack(const DecisionPath& top) {
    for (int i = 0; i < top.length; ++i) {
      //printf("    Resetting decision stack node %d to %s\n", top.nodes[i]->num, logicval_to_str(L_X));
        LogicVal v = L_X;
        top.nodes[i]->d_value = L_X;
        top.nodes[i]->force_d_value = false;
        top.nodes[i]->visited = false;
    }
}

bool is_j_frontier(NSTRUC *node) {
    if (node->d_value == L_X) return false;
    for (int i = 0; i < node->fin; ++i) {
        if (node->unodes[i]->d_value == L_X) {
            return true;
        }
    }
    return false;
}

bool backtrace_input(NSTRUC *node) {
    backtrace_calls++;
    if (backtrace_calls > get_max_level()) return false;
 //printf("    Backtracing node %d with value %s\n", node->num, logicval_to_str(node->d_value));
    enqueue(node);
    bool updated = false;
    bool has_ctrl = false;
    LogicVal ctrl = L_X;
    bool inv = false;
    if (node->type == NOT || node->type == NAND || node->type == NOR) {
        inv = true;
    }
    if (node->d_value == L_D || node->d_value == L_1) {
        ctrl = inv ? L_0:L_1;
    } else if (node->d_value == L_0 || node->d_value == L_DB) {
        ctrl = inv ? L_1:L_0;
    }
    if (current_path_len < MAX_PATH_LEN) {
        current_path_nodes[current_path_len] = node;
        current_path_values[current_path_len] = node->d_value;
        current_path_len++;
    } else {
     //printf("Warning: Exceeded max path length in backtrace.\n");
    }
    if (node->type == IPT){
        return false;
    } else {
        int best_index = -1;
        int best_cc = INT_MAX;
        for (int j = 0; j < node->fin; ++j) {
            NSTRUC *input = node->unodes[j];
         //printf("Node: %d, Value: %s, J? %d, Visited? %d \n", input->num, logicval_to_str(input->d_value), is_j_frontier(node), input->visited);
            if ((input->d_value == L_X) && is_j_frontier(node) && (!input->visited)) {
        //         int cc = (ctrl == L_0) ? input->cc0 : input->cc1;
        //         if (cc < best_cc) {
        //             best_cc = cc;
        //             best_index = j;
        //         }
        //     }
        // }
                if (jfrontier_mode == "v0") {
                    // v0: pick fanin with lowest controllability
                    int cc = (ctrl == L_0) ? input->cc0 : input->cc1;
                    if (cc < best_cc) {
                        best_cc = cc;
                        best_index = j;
                        // printf("Lowest controllability found: node %d with cc = %d\n", input->num, cc);
                    }
                } else {
                    best_index = j;
                    break;
                }
            }
        }



        if (best_index != -1) {
            NSTRUC *chosen = node->unodes[best_index];
            chosen->d_value = ctrl;
          //printf("        Backtraced node %d to %s for node %d = %s (%s)\n", chosen->num, logicval_to_str(ctrl), node->num, logicval_to_str(node->d_value), nodetype_to_str(node->type));
            updated = true;
            updated |= backtrace_input(chosen);
        }
        return updated;
    }
}
// READ ../auto-tests-phase3/ckts/c3.ckt
// PODEM 4 0 ../auto-tests-phase3/outputs/dalg/c3_dalg_4_0.out
// QUIT


bool backtrace(NSTRUC *node) {
    backtrace_calls = 0;
    bool any_update = false;
    current_path_len = 0;
  //printf("Starting backtrace...\n");
    // bool backtraced_this_pass[Nnodes];
    std::vector<bool> backtraced_this_pass(Nnodes, false);
    // memset(backtraced_this_pass, 0, sizeof(backtraced_this_pass));
    bool updated = true;
  //printf("    Backtracing node %d with value %s\n", node->num, logicval_to_str(node->d_value));            
    updated = backtrace_input(node);
    any_update |= updated;
    if (current_path_len > 0) {
        push_decision_path(current_path_nodes, current_path_values, current_path_len);
    }
    if (!any_update) {
      //printf("No updates were made during backtrace.\n");
    }
    for (int i = current_path_len-1; i >= 0; --i) {
        NSTRUC* n = current_path_nodes[i];
        for (int j = 0; j < n->fout; ++j) {
            if (n->dnodes[j]->fin > 1) {
               //printf("Set node %d to visited\n", n->num);
                n->visited = true;
                goto done_marking;
            }
        }
    }
    done_marking:;
    return any_update;
}


LogicVal get_controlling_value(int type) {
    switch (type) {
        case AND: case NAND: return L_0;
        case OR:  case NOR:  return L_1;
        default:             return L_1;
    }
}

bool is_d_frontier(NSTRUC *node) {
    if (!(node->d_value == L_X)) return false;
    for (int i = 0; i < node->fin; ++i) {
        if (node->unodes[i]->d_value == L_D || node->unodes[i]->d_value == L_DB) {
            return true;
        }
    }
    return false;
}

int fault_num = -1, fault_val = -1;

bool check_output() {
    fault_reached_output = false;
    for (int p = 0; p < Npo; ++p) {
        if (Poutput[p]->d_value == L_D || Poutput[p]->d_value == L_DB) {
            fault_reached_output = true;
          //printf("After Forward Implication:\n");
            // for (int i = 0; i < Nnodes; ++i) {
            //  //printf("  Node[%d]: d_value = %s\n", Node[i].num, logicval_to_str(Node[i].d_value));
            // }
          //printf("Fault propagated to primary output %d with value %s\n", Poutput[p]->num, logicval_to_str(Poutput[p]->d_value));
        }
    }
    return fault_reached_output;
}

bool fault_active() {
    LogicVal original = fault_node->d_value;
    LogicVal normalized;
    if (fault_node->d_value == L_D) {
        normalized = L_1;
    } else if (fault_node->d_value == L_DB) {
        normalized = L_0;
    } else {
      //printf("Error: Fault node is not D or D'\n");
        return false;
    }
    if (fault_node->type == IPT) {
        return true;
    }
    fault_node->force_d_value = false;
    LogicVal eval = evaluate_d_gate(fault_node);
    fault_node->d_value = original;
    fault_node->force_d_value = true;
  //printf("Faulty value: %s, Normalized value: %s, Evaluated value: %s\n", logicval_to_str(original), logicval_to_str(normalized), logicval_to_str(eval));
    return (eval == normalized);
}

// bool forward_imply() {
//     bool result = true;
//     for (int i = 0; i < Npi; ++i) {
//         Pinput[i]->force_d_value = true;
//     }
//     reset_non_forced();
//     for (int level = 0; level <= get_max_level(); ++level) {
//         for (int i = 0; i < Nnodes; ++i) {
//             if (Node[i].level == level && Node[i].fin > 0) {
//                 Node[i].d_value = evaluate_d_gate(&Node[i]); 
//             }
//         }
//     }
//     for (int i = 0; i < Npi; ++i) {
//         Pinput[i]->force_d_value = false;
//     }
//     return fault_active();
// }

bool forward_imply() {
    while (!event_queue.empty()) {
     //printf("Processing node %d\n", event_queue.top()->num);
        NSTRUC* node = event_queue.top(); event_queue.pop();
        enqueued[node->index] = false;
        // node->visited = false;
        for (int i = 0; i < node->fout; ++i) {
            NSTRUC* fanout = node->dnodes[i];
            if (!fanout || fanout->fin == 0) continue;
            LogicVal new_val = evaluate_d_gate(fanout);
            if (fanout->d_value != new_val) {
                fanout->d_value = new_val;
             //printf("    Forward Implying node %d to %s\n", fanout->num, logicval_to_str(new_val));
                enqueue(fanout);
            }
        }
    }
    std::fill(enqueued.begin(), enqueued.end(), false);
    return fault_active();
}

// READ ../auto-tests-phase3/ckts/c3.ckt
// PODEM 10 0 ../auto-tests-phase3/outputs/dalg/c3_dalg_10_0.out
// QUIT

void backward_imply() {
    for (int level = get_max_level(); level >= 0; --level) {
        for (int i = 0; i < Nnodes; ++i) {
            NSTRUC* node = &Node[i];
            if (node->level != level || node->fin == 0 || node->d_value == L_X) continue;
            switch (node->type) {
                case NOT:
                    if (node->unodes[0]->d_value == L_X) {
                        node->unodes[0]->d_value = invert_logicval(node->d_value);
                    }
                    break;
                case BRCH:
                case BUFFER:
                    if (node->unodes[0]->d_value == L_X) {
                        node->unodes[0]->d_value = node->d_value;
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

bool set_fault_site(int fault_node_num, int val) {
    for (int i = 0; i < Nnodes; ++i) {
        if (Node[i].num == fault_node_num) {
            fault_node = &Node[i];
            fault_val = val;
          //printf("Fault site set to node %d with value %d\n", fault_node_num, val);
            return true;
        }
    }
  //printf("Error: Fault node number %d not found.\n", fault_node_num);
    return false;
}


std::pair<int, LogicVal> objective() {
    if (fault_node->d_value == L_X) {
        LogicVal vbar = (fault_val == 0) ? L_1 : L_0;
        inject_fault(fault_node->num, fault_val);
      //printf("Injecting fault at node %d with value %d\n", fault_node->num, fault_val);
        // fault_node->set_objective = true;
        return { fault_node->num, vbar };
    }
    if (!fault_active()) {
        bool all_inputs_assigned = true;
        for (int j = 0; j < fault_node->fin; ++j) {
            if (fault_node->unodes[j]->d_value == L_X) {
                all_inputs_assigned = false;
                break;
            }
        }
        if (all_inputs_assigned) {
          //printf("Fault cannot be activated: inputs fully assigned.\n");
            return { -1, L_X };
        }
        return { fault_node->num, (fault_val == 0) ? L_1 : L_0 };
        // if (!fault_node->set_objective) {
        //     fault_node->set_objective = true;
        //     return { fault_node->num, (fault_val == 0) ? L_1 : L_0 };
        // } else {
        //     return { -1, L_X };
        // }
    }
    // int best_node_num = -1;
    // LogicVal best_val = L_X;
    // int best_d = 1e9;
    // int best_index = -1;
    // int fanin_index = -1;
    // NSTRUC *tmp_node;
    // for (int i = 0; i < Nnodes; ++i) {
    //     NSTRUC *node = &Node[i];
    //    //printf("%d, %s, %d\n", node->num, logicval_to_str(node->d_value), is_d_frontier(node));
    //     if (node->d_value == L_X && is_d_frontier(node)) {
    //         for (int j = 0; j < node->fin; ++j) {
    //             NSTRUC *fanin = node->unodes[j];
    //             if (fanin->d_value == L_X) {
    //                 LogicVal controlling = get_controlling_value(node->type);
    //                 LogicVal cbar = invert_logicval(controlling);
    //                 int cc = (cbar == L_0) ? fanin->cc0:fanin->cc1;
    //                 int d = fanin->co;

    //                 if (d < best_d) {
    //                     best_index = i;
    //                     fanin_index = j;
    //                     best_d = d;
    //                     best_node_num = fanin->num;
    //                     best_val = cbar;
    //                     tmp_node = node;
    //                 }
    //             }
    //         }
    //     }
    // }
    // if (best_node_num != -1) {
    //   //printf("Found D-frontier node %d with value %s\n", best_node_num, logicval_to_str(best_val));
    //     Node[best_index].unodes[fanin_index]->d_value = best_val;
    //     return { best_node_num, best_val };
    // }
    // return { -1, L_X };


    int best_metric = (dfrontier_mode == "nh" || dfrontier_mode == "lh") ? -1 : 1e9;
    int best_index = -1;
    int fanin_index = -1;
    int best_node_num = -1;
    LogicVal best_val = L_X;
    NSTRUC* tmp_node = nullptr;
    
    for (int i = 0; i < Nnodes; ++i) {
        NSTRUC *node = &Node[i];
        if (node->d_value == L_X && is_d_frontier(node)) {
            for (int j = 0; j < node->fin; ++j) {
                NSTRUC *fanin = node->unodes[j];
                if (fanin->d_value == L_X) {
                    LogicVal controlling = get_controlling_value(node->type);
                    LogicVal cbar = invert_logicval(controlling);
    
                    if (dfrontier_mode == "") {
                        // DEFAULT CASE: pick first one you find
                        fanin->d_value = cbar;
                        return { fanin->num, cbar };
                    }
    
                    // Otherwise (dfrontier_mode is nl, nh, lh, cc)
                    int metric = 0;
                    if (dfrontier_mode == "nl") {
                        metric = fanin->num; // Node number (lower better)
                    } else if (dfrontier_mode == "nh") {
                        metric = fanin->num; // Node number (higher better)
                    } else if (dfrontier_mode == "lh") {
                        metric = node->level; // Level (higher better)
                    } else if (dfrontier_mode == "cc") {
                        metric = (cbar == L_0) ? fanin->cc0 : fanin->cc1; // Observability (lower better)
                    }
    
                    bool better = false;
                    if (dfrontier_mode == "nh" || dfrontier_mode == "lh") {
                        better = (metric > best_metric);
                    } else {
                        better = (metric < best_metric);
                    }
    
                    if (better) {
                        best_index = i;
                        fanin_index = j;
                        best_metric = metric;
                        best_node_num = fanin->num;
                        best_val = cbar;
                        tmp_node = node;
                        // if (dfrontier_mode == "nl") {
                        //     printf("Lowest node number found: %d\n", metric);
                        // } else if (dfrontier_mode == "nh") {
                        //     printf("Highest node number found: %d\n", metric);
                        // } else if (dfrontier_mode == "lh") {
                        //     printf("Highest level found: %d\n", metric);
                        // } else if (dfrontier_mode == "cc") {
                        //     printf("Lowest observability found: %d\n", metric);
                        // }
                    }
                }
            }
        }
    }
    
    if (best_node_num != -1) {
        Node[best_index].unodes[fanin_index]->d_value = best_val;
        return { best_node_num, best_val };
    }
    
    return { -1, L_X };
    

}


// READ ../auto-tests-phase3/ckts/c3.ckt
// PODEM 4 0 ../auto-tests-phase3/outputs/dalg/c3_dalg_4_0.out
// QUIT
// static std::unordered_set<std::pair<int, LogicVal>, pair_hash> tried_assignments;
std::unordered_set<std::string> tried_patterns;
static int last_obj_node = -1;
static LogicVal last_obj_val = L_X;

bool podem_recursive() {
// bool podem_recursive(std::unordered_set<std::pair<int, LogicVal>, pair_hash>& tried_assignments) {
    // static std::unordered_set<std::pair<int, LogicVal>, pair_hash> tried_assignments;
    podem_rec_calls++;
    // printf("Recursion Depth: %lld\n", podem_rec_calls);
    // if (podem_rec_calls > 25*Nnodes) {
    if (podem_rec_calls > 2*Npi) {
        return false;
    }
    bool inconsistent;
  //printf("Starting PODEM...\n");
    std::pair<int, LogicVal> obj = objective();        
    if (!fault_active() && obj.first == -1) {
        // podem_rec_calls--;
        return false;           
    }
    int node_num = obj.first;
    if (node_num == -1) {
      //printf("No objective found.\n");
    //   podem_rec_calls--;
        return false;
    }
    if (obj.first != last_obj_node || obj.second != last_obj_val) {
        for (int i = 0; i < Nnodes; ++i)
            Node[i].visited = false;
        last_obj_node = obj.first;
        last_obj_val = obj.second;
    }
    LogicVal logic_val = obj.second;
  //printf("Objective: Node %d, Value %s\n", node_num, logicval_to_str(logic_val));
    if (node_num == -1 || logic_val == L_X) return false;
    NSTRUC* node = nullptr;
    for (int i = 0; i < Nnodes; ++i) {
        if (Node[i].num == node_num) {
            node = &Node[i];
            break;
        }
    }
    if (!node) {
        // podem_rec_calls--;
        return false;
    }
    push_state();
  //printf("Trying to backtrace node %d with value %s\n", node->num, logicval_to_str(logic_val));
    if (!x_path_check(node)) {
        // printf("X-path check failed.\n");
        // podem_rec_calls--;
        return false;
      }
    bool backtraced = backtrace(node);
    backward_imply();

  //printf("Final Primary Input values after backward imply:\n");
    for (int i = 0; i < Npi; ++i) {
      //printf("  PI %d (Node %d): %s\n", i, Pinput[i]->num, logicval_to_str(Pinput[i]->d_value));
    }
    inconsistent = !forward_imply();
    check_output();


    // Build full PI vector pattern
    std::string pattern;
    for (int i = 0; i < Npi; ++i) {
        switch (Pinput[i]->d_value) {
            case L_0:  pattern += '0'; break;
            case L_1:  pattern += '1'; break;
            case L_D:  pattern += '1'; break;
            case L_DB: pattern += '0'; break;
            default:   pattern += 'x'; break;
        }
    }
    if (tried_patterns.count(pattern)) {
        // podem_rec_calls--;
        return false;
    }
    tried_patterns.insert(pattern);


    if (fault_reached_output && !inconsistent) {
      //printf("Fault reached output after backtrace and forward implication.\n");
        return true;
    } else {
      //printf("Failure due to inconsistency or fault not propagated.\n");
    }
    ////printf("After Forward Implication:\n");
    // for (int i = 0; i < Nnodes; ++i) {
    //  //printf("  Node[%d]: d_value = %s\n", Node[i].num, logicval_to_str(Node[i].d_value));
    // }
    // for (int i = 0; i < Nnodes; ++i)
    //     Node[i].visited = false;
    
    if (podem_recursive()) return true;
    pop_state();
    push_state();

    // First failure flip the value 
    LogicVal opposite = invert_logicval(logic_val);             //Need to invert primary input value and not node value. 
  //printf("1st Failure: Flipping Most Recent Decision Path:\n");
    // top = pop_decision_path();
    DecisionPath top = pop_decision_path();
    flip_decision_stack(top);
    backward_imply();
    // if (!x_path_check(node)) {
    //     //printf("X-path check failed.\n");
    //     // podem_rec_calls--;
    //     return false;
    //   }
  //printf("Final Primary Input values after backward imply:\n");
    // for (int i = 0; i < Npi; ++i) {
    //   //printf("  PI %d (Node %d): %s\n", i, Pinput[i]->num, logicval_to_str(Pinput[i]->d_value));
    // }

    inconsistent = !forward_imply();
    check_output();

    if (fault_reached_output && !inconsistent) {
      //printf("Fault reached output after backtrace and forward implication.\n");
        return true;
    }
    // for (int i = 0; i < Nnodes; ++i)
    //     Node[i].visited = false;    
    // podem_rec_calls = 0;
    if (podem_recursive()) return true;
    pop_state();

    // Second failure assign x 
  //printf("2nd Failure:\n");
    reset_decision_stack(top);
    // podem_rec_calls--;
    return false;
}

bool podem_recursive_top() {
        // printf("Recursion Depth: %lld\n", podem_rec_calls);
        bool inconsistent;
      //printf("Starting PODEM...\n");
        std::pair<int, LogicVal> obj = objective();        
        if (!fault_active() && obj.first == -1) {
            // podem_rec_calls--;
            return false;           
        }
        int node_num = obj.first;
        if (node_num == -1) {
          //printf("No objective found.\n");
        //   podem_rec_calls--;
            return false;
        }
        if (obj.first != last_obj_node || obj.second != last_obj_val) {
            for (int i = 0; i < Nnodes; ++i)
                Node[i].visited = false;
            last_obj_node = obj.first;
            last_obj_val = obj.second;
        }
        LogicVal logic_val = obj.second;
      //printf("Objective: Node %d, Value %s\n", node_num, logicval_to_str(logic_val));
        if (node_num == -1 || logic_val == L_X) return false;
        NSTRUC* node = nullptr;
        for (int i = 0; i < Nnodes; ++i) {
            if (Node[i].num == node_num) {
                node = &Node[i];
                break;
            }
        }
        if (!node) {
            // podem_rec_calls--;
            return false;
        }
        push_state();
      //printf("Trying to backtrace node %d with value %s\n", node->num, logicval_to_str(logic_val));
        bool backtraced = backtrace(node);
        backward_imply();
    
      //printf("Final Primary Input values after backward imply:\n");
        for (int i = 0; i < Npi; ++i) {
        }
        inconsistent = !forward_imply();
        check_output();
    
    
        // Build full PI vector pattern
        std::string pattern;
        for (int i = 0; i < Npi; ++i) {
            switch (Pinput[i]->d_value) {
                case L_0:  pattern += '0'; break;
                case L_1:  pattern += '1'; break;
                case L_D:  pattern += '1'; break;
                case L_DB: pattern += '0'; break;
                default:   pattern += 'x'; break;
            }
        }
        if (tried_patterns.count(pattern)) {
            // podem_rec_calls--;
            return false;
        }
        tried_patterns.insert(pattern);
        if (fault_reached_output && !inconsistent) {
            return true;
        } else {
        }
        podem_rec_calls = 0;
        if (podem_recursive()) return true;
        pop_state();
        push_state();
    
        // First failure flip the value 
        LogicVal opposite = invert_logicval(logic_val);             //Need to invert primary input value and not node value. 
      //printf("1st Failure: Flipping Most Recent Decision Path:\n");
        // top = pop_decision_path();
        DecisionPath top = pop_decision_path();
        flip_decision_stack(top);
        backward_imply();
        // if (!x_path_check(node)) {
        //     //printf("X-path check failed.\n");
        //     // podem_rec_calls--;
        //     return false;
        //   }
      //printf("Final Primary Input values after backward imply:\n");
        // for (int i = 0; i < Npi; ++i) {
        //   //printf("  PI %d (Node %d): %s\n", i, Pinput[i]->num, logicval_to_str(Pinput[i]->d_value));
        // }
    
        inconsistent = !forward_imply();
        check_output();
    
        if (fault_reached_output && !inconsistent) {
          //printf("Fault reached output after backtrace and forward implication.\n");
            return true;
        }
        // for (int i = 0; i < Nnodes; ++i)
        //     Node[i].visited = false;    
        // podem_rec_calls = 0;
        if (podem_recursive()) return true;
        pop_state();
    
        // Second failure assign x 
      //printf("2nd Failure:\n");
        reset_decision_stack(top);
        // podem_rec_calls--;
        return false;
    }


void podem() {
    tried_patterns.clear();
    podem_rec_calls = 0;
    char saved_cp[MAXLINE];
    strncpy(saved_cp, cp, MAXLINE);
    char dummy_cp[] = "temp_scoap.txt";
    cp = dummy_cp;
    scoap();
    cp = saved_cp;
    char output_file[MAXLINE];
    if (sscanf(cp, "%d %d %s", &fault_num, &fault_val, output_file) != 3) {
      //printf("Usage: PODEM <node> <0|1> <output_file>\n");
        return;
    }
    FILE *fp = fopen(output_file, "w");
    if (!fp) {
      //printf("Cannot open output file: %s\n", output_file);
        return;
    }
    // fprintf(fp, "\n");
    for (int i = 0; i < Npi; ++i) {
        fprintf(fp, "%d", Pinput[i]->num);
        if (i < Npi - 1) fprintf(fp, ",");
    }
    fprintf(fp, "\n");
    reset_d_values();
    enqueued.resize(Nnodes, false);
    for (int i = 0; i < Nnodes; ++i) {
        enqueue(&Node[i]);
    }
    set_fault_site(fault_num, fault_val);
    check_output();
    podem_recursive_top();
    if (fault_reached_output) {
        for (int pi = 0; pi < Npi; ++pi) {
            switch (Pinput[pi]->d_value) {
                case L_0: fprintf(fp, "0"); break;
                case L_D: fprintf(fp, "1"); break;
                case L_1: fprintf(fp, "1"); break;
                case L_DB: fprintf(fp, "0"); break;
                default:  fprintf(fp, "x"); break;
                }
                if (pi < Npi - 1) fprintf(fp, ",");
        }
        fprintf(fp, "\n");
       //printf("Primary Input values After Successful PODEM:\n");
        // for (int pi = 0; pi < Npi; ++pi) {
        //     switch (Pinput[pi]->d_value) {
        //         case L_0://printf("0"); break;
        //         case L_D://printf("1"); break;
        //         case L_1://printf("1"); break;
        //         case L_DB://printf("0"); break;
        //         default: //printf("x"); break;
        //         }
        //         if (pi < Npi - 1)//printf(",");
        // }
       //printf("\n");
    } else {
       //printf("No solution found.\n");
    }
    fclose(fp);
}

// READ ../auto-tests-phase3/ckts/c3.ckt
// PODEM 1 0 ../auto-tests-phase3/outputs/podem/c3_podem_1_0.out
// QUIT

// READ ../auto-tests-phase3/ckts/c3.ckt
// PODEM 22 0 ../auto-tests-phase3/outputs/dalg/c3_dalg_22_0.out
// QUIT

// READ ../auto-tests-phase3/ckts/c17.ckt
// PODEM 11 1 ../auto-tests-phase3/outputs/dalg/c17_dalg_11_1.out
// QUIT




// READ ../auto-tests-phase3/ckts/c17.ckt
// TPG PODEM DFS my_patterns.tp
// TPG DALG DFS my_patterns.tp

// READ ../auto-tests-phase3/ckts/c1.ckt
// TPG PODEM DFS my_patterns.tp -fl rfl

// READ ../ckts_all/c6288.ckt
// TPG PODEM PFS my_patterns.tp -rtp v1 30
// TPG PODEM PFS my_patterns.tp -rtp v2 0.00001

// READ ../ckts_all/c432.ckt
// TPG PODEM PFS my_patterns.tp -rtp v1 30

// READ ../auto-tests-phase3/ckts/c17.ckt
// DALG 1 0 ../auto-tests-phase3/outputs/dalg/c17_dalg_1_0.out