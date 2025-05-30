// #include "dalg.h"
// #include "atpg.h"
// #include "levelizer.h"
// #include "scoap.h"

// bool justify_input(NSTRUC *node);

// void propagate_fault(int node_num) {
//     NSTRUC *node = &Node[node_num];
//     if (node->visited) return;
//     node->visited = true;
//     printf("Fault propagation starting at node %d with value %s\n", node->num, logicval_to_str(node->d_value));
//     int node_indices[MAX_NODES];
//     LogicVal expected_vals[MAX_NODES];
//     int total_fanouts = node->fout;
//     int max_combinations = (1 << total_fanouts);
//     int masks[(1 << total_fanouts)];
//     int num_masks = 0;
//     for (int mask = 1; mask < max_combinations; ++mask) {
//         if (mask == 1) continue;
//         masks[num_masks++] = mask;
//     }
//     for (int i = 0; i < num_masks - 1; ++i) {
//         for (int j = i + 1; j < num_masks; ++j) {
//             if (__builtin_popcount(masks[j]) > __builtin_popcount(masks[i])) {
//                 int tmp = masks[i];
//                 masks[i] = masks[j];
//                 masks[j] = tmp;
//             }
//         }
//     }
//     for (int m = 0; m < num_masks; ++m) {
//         int mask = masks[m];        
//         if (mask == 1) continue;
//         int count = 0;
//         for (int i = 0; i < total_fanouts; ++i) {
//             if (mask & (1 << i)) {
//                 NSTRUC *fanout = node->dnodes[i];
//                 for (int j = 0; j < Nnodes; ++j) {
//                     if (&Node[j] == fanout) {
//                         node_indices[count] = fanout->num;
//                         expected_vals[count] = node->d_value;
//                         count++;
//                         if (count >= MAX_NODES) {
//                             printf("Too many fanouts in mask = %d\n", mask);
//                             exit(1);
//                         }
//                         break;
//                     }
//                 }
//             }
//         }
//         if (count > 0) {
//             push_dfrontier(node_indices, expected_vals, count);
//         }
//     }
//     for (int i = 0; i < node->fout; ++i) {
//         NSTRUC *output_node = node->dnodes[i];
//         printf("    Output of Node %d is Node %d\n", node->num, output_node->num);
//         if (node->d_value == L_D || node->d_value == L_DB) {
//             if (output_node->type == AND || output_node->type == NAND) {
//                 for (int j = 0; j < output_node->fin; ++j) {
//                     if (output_node->unodes[j]->d_value != L_D && output_node->unodes[j]->d_value != L_DB) {
//                         if (output_node->unodes[j] != node) {
//                             output_node->unodes[j]->d_value = L_1;
//                             output_node->unodes[j]->force_d_value = true;
//                             printf("    Setting node %d to 1\n", output_node->unodes[j]->num);
//                         }
//                     }
//                 }
//             }
//             else if (output_node->type == OR || output_node->type == NOR) {
//                 for (int j = 0; j < output_node->fin; ++j) {
//                     if (output_node->unodes[j]->d_value != L_D && output_node->unodes[j]->d_value != L_DB) {
//                         if (output_node->unodes[j] != node) {
//                             output_node->unodes[j]->d_value = L_0;
//                             output_node->unodes[j]->force_d_value = true;
//                             printf("    Setting node %d to 0\n", output_node->unodes[j]->num);
//                         }
//                     }
//                 }
//             }



//             else if (output_node->type == XOR || output_node->type == XNOR) {
//                 int num_assigned = 0;
//                 int sum = 0;
//                 for (int j = 0; j < output_node->fin; ++j) {
//                     if (output_node->unodes[j]->d_value == L_1 || output_node->unodes[j]->d_value == L_D) {
//                         sum += 1;
//                     }
//                 }
//                 for (int j = 0; j < output_node->fin; ++j) {
//                     if (output_node->unodes[j]->d_value == L_X && output_node->unodes[j] != node) {
//                         if (((output_node->type == XOR && output_node->d_value == L_D) || (output_node->type == XNOR && output_node->d_value == L_DB))) {
//                             output_node->unodes[j]->d_value = (sum % 2 == 0) ? L_1 : L_0;
//                         } else {
//                             output_node->unodes[j]->d_value = (sum % 2 == 0) ? L_0 : L_1;
//                         }
//                         output_node->unodes[j]->force_d_value = true;
//                         printf("    Setting node %d to %s (to maintain correct parity for XOR/XNOR)\n",
//                                output_node->unodes[j]->num, logicval_to_str(output_node->unodes[j]->d_value));
//                         break;
//                     }
//                 }
//             }            




//             output_node->force_d_value = false;
//             output_node->d_value = evaluate_d_gate(output_node);
//             printf("    Evaluated node %d to %s\n", output_node->num, logicval_to_str(output_node->d_value));
//             bool is_po = false;
//             for (int p = 0; p < Npo; ++p) {
//                 if (Poutput[p] == output_node) {
//                     is_po = true;
//                     break;
//                 }
//             }
//             if ((output_node->d_value == L_D || output_node->d_value == L_DB) && is_po) {
//                 printf("Fault propagated to **primary output** node %d with value %s\n", output_node->num, logicval_to_str(output_node->d_value));
//                 fault_reached_output = true;
//                 break;
//             }
//             int index = -1;
//             for (int i = 0; i < Nnodes; ++i) {
//                 if (&Node[i] == output_node) {
//                     index = i;
//                     break;
//                 }
//             }
//             propagate_fault(index);
//             if (!fault_reached_output) {
//                 printf("    Backtracking from fanout node %d (didn't lead to PO with D/D')\n", output_node->num);
//             } else {
//                 break;
//             }

//         }
//     }
// }

// bool justify_and(NSTRUC *node) {
//     bool updated = false;
//     bool has_zero = false;
//     LogicVal Old;
//     LogicVal Old_;
//     LogicVal New;
//     LogicVal New_;
//     LogicVal set;
//     if (node->d_value == L_D || node->d_value == L_1) {
//         set = L_1;
//         for (int j = 0; j < node->fin; ++j) {
//             NSTRUC *input = node->unodes[j];
//             if (node->d_value == L_1 && (input->d_value == L_D || input->d_value == L_DB || input->d_value == L_0)) {
//                 inconsistent = true;
//                 printf("    Inconsistency: %s output is %s but input is %s\n", nodetype_to_str(node->type), logicval_to_str(node->d_value), logicval_to_str(input->d_value));
//                 updated = false;
//                 return updated;
//             }
//         }
//         for (int j = 0; j < node->fin; ++j) {
//             NSTRUC *input = node->unodes[j];
//             if (input->d_value == L_X) {
//                 input->d_value = set;
//                 has_zero = true;
//                 printf("        Justified node %d to %s for node %d = %s (%s)\n", input->num, logicval_to_str(set), node->num, logicval_to_str(node->d_value), nodetype_to_str(node->type));
//                 updated = true;
//                 updated |= justify_input(input);
//                 if (inconsistent) {
//                     input->d_value = d_not(input->d_value);
//                     printf("    Backtracking on node %d (reset to %s)\n", input->num, logicval_to_str(input->d_value));
//                     return updated;
//                 } else {
//                     continue;
//                 }
//             }
//             if (input->d_value == L_0) {
//                 inconsistent = true;
//                 printf("    Inconsistency: %s output is %s but input is %s\n", nodetype_to_str(node->type), logicval_to_str(node->d_value), logicval_to_str(input->d_value));
//             }
//         }
//     } else if (node->d_value == L_0 || node->d_value == L_DB) {
//         for (int j = 0; j < node->fin; ++j) {
//             NSTRUC *input = node->unodes[j];
//             if (input->d_value == L_X && !has_zero) {
//                 input->d_value = L_0;
//                 printf("        Justified node %d to 0 for node %d = %s (%s)\n",
//                        input->num, node->num, logicval_to_str(node->d_value), nodetype_to_str(node->type));
//                 updated = true;
//                 updated |= justify_input(input);
//                 if (inconsistent) {
//                     input->d_value = d_not(input->d_value);
//                     inconsistent = false;
//                     has_zero = false;
//                     printf("    Backtracking on node %d (reset to %s)\n", input->num, logicval_to_str(input->d_value));
//                     continue;
//                 } else {
//                     has_zero = true;
//                     continue;
//                 }
//             } else if (input->d_value == L_0) {
//                 has_zero = true;
//                 continue;
//             }
//         }
//         if (!has_zero) {
//             inconsistent = true;
//             printf("    Inconsistency: %s output is 0 but no input is 0\n", nodetype_to_str(node->type));
//         }
//     }
//     node->force_d_value = false;
//     New = evaluate_d_gate(node);
//     node->force_d_value = true;
//     Old = node->d_value;
//     Old_ = Old;
//     New_ = New;
//     if (Old == L_D || Old == L_DB) {
//         if (New_ == L_D) {
//             New_ = L_1;
//         } else if (New_ == L_DB) {
//             New_ = L_0;
//         }
//         if (Old == L_D) {
//             Old_ = L_1;
//         } else if (Old == L_DB) {
//             Old_ = L_0;
//         }
//     }
//     if (logicval_to_str(Old_) != logicval_to_str(New_)) {
//         inconsistent = true;
//         printf("    Inconsistency: %s output is %s but input suggests %s or %s\n", nodetype_to_str(node->type), logicval_to_str(Old), logicval_to_str(New), logicval_to_str(New_));    }
//     return updated;
// }

// bool justify_branch(NSTRUC *node) {
//     bool updated = false;
//     LogicVal expected;
//     if (node->d_value == L_DB || node->d_value == L_D || node->d_value == L_1 || node->d_value == L_0) {
//         if (node->d_value == L_D || node->d_value == L_1) {
//             expected = L_1;
//         } else if (node->d_value == L_DB || node->d_value == L_0) {
//             expected = L_0;
//         } else {
//             expected = node->d_value;
//         }
//         for (int j = 0; j < node->fin; ++j) {
//             NSTRUC *input = node->unodes[j];

//             if (input->d_value == L_X) {
//                 input->d_value = expected;
//                 printf("        Justified node %d to %s for node %d = %s (%s)\n", input->num, logicval_to_str(expected), node->num, logicval_to_str(node->d_value), nodetype_to_str(node->type));
//                 updated |= justify_input(input);

//                 if (inconsistent) {
//                     input->d_value = d_not(input->d_value);
//                     printf("    Backtracking on node %d (reset to %s)\n", input->num, logicval_to_str(input->d_value));
//                     return updated;
//                 } else {
//                     continue;
//                 }
//             } else if (input->d_value != expected) {
//                 inconsistent = true;
//                 printf("    Inconsistency at node %d: expected %s, found %s (%s)\n",
//                        input->num, logicval_to_str(expected), logicval_to_str(input->d_value), nodetype_to_str(node->type));
//                 return updated;
//             }
//         }

//     }
//     return updated;
// }

// bool justify_nand(NSTRUC *node) {
//     bool updated = false;
//     bool has_zero = false;
//     LogicVal Old;
//     LogicVal Old_;
//     LogicVal New;
//     LogicVal New_;
//     LogicVal set;
//     if (node->d_value == L_DB || node->d_value == L_0) {
//         set = L_1;
//         for (int j = 0; j < node->fin; ++j) {
//             NSTRUC *input = node->unodes[j];
//             if (node->d_value == L_0 && (input->d_value == L_D || input->d_value == L_DB || input->d_value == L_0)) {
//                 inconsistent = true;
//                 printf("    Inconsistency: %s output is %s but input is %s\n", nodetype_to_str(node->type), logicval_to_str(node->d_value), logicval_to_str(input->d_value));
//                 updated = false;
//                 return updated;
//             }
//         }
//         for (int j = 0; j < node->fin; ++j) {
//             NSTRUC *input = node->unodes[j];
//             if (input->d_value == L_X) {
//                 input->d_value = set;
//                 has_zero = true;
//                 printf("        Justified node %d to %s for node %d = %s (%s)\n", input->num, logicval_to_str(set), node->num, logicval_to_str(node->d_value), nodetype_to_str(node->type));
//                 updated = true;
//                 updated |= justify_input(input);
//                 if (inconsistent) {
//                     input->d_value = d_not(input->d_value);
//                     printf("    Backtracking on node %d (reset to %s)\n", input->num, logicval_to_str(input->d_value));
//                     return updated;
//                 } else {
//                     continue;
//                 }
//             }
//             if (input->d_value == L_0) {
//                 inconsistent = true;
//                 printf("    Inconsistency: %s output is %s but input is %s\n", nodetype_to_str(node->type), logicval_to_str(node->d_value), logicval_to_str(input->d_value));
//             }
//         }
//     } else if (node->d_value == L_1 || node->d_value == L_D) {
//         for (int j = 0; j < node->fin; ++j) {
//             NSTRUC *input = node->unodes[j];
//             if (input->d_value == L_X && !has_zero) {
//                 input->d_value = L_0;
//                 printf("        Justified node %d to 0 for node %d = %s (%s)\n",
//                        input->num, node->num, logicval_to_str(node->d_value), nodetype_to_str(node->type));
//                 updated = true;
//                 updated |= justify_input(input);
//                 if (inconsistent) {
//                     input->d_value = d_not(input->d_value);
//                     inconsistent = false;
//                     has_zero = false;
//                     printf("    Backtracking on node %d (reset to %s)\n", input->num, logicval_to_str(input->d_value));
//                     continue;
//                 } else {
//                     has_zero = true;
//                     continue;
//                 }
//             } else if (input->d_value == L_0) {
//                 has_zero = true;
//                 continue;
//             }
//         }
//         if (!has_zero) {
//             inconsistent = true;
//             printf("    Inconsistency: %s output is %s but no input is 0\n", nodetype_to_str(node->type), logicval_to_str(node->d_value));
//         }
//     }
//     node->force_d_value = false;
//     New = evaluate_d_gate(node);
//     node->force_d_value = true;
//     Old = node->d_value;
//     Old_ = Old;
//     New_ = New;
//     if (Old == L_D || Old == L_DB) {
//         if (New_ == L_D) {
//             New_ = L_1;
//         } else if (New_ == L_DB) {
//             New_ = L_0;
//         }
//         if (Old == L_D) {
//             Old_ = L_1;
//         } else if (Old == L_DB) {
//             Old_ = L_0;
//         }
//     }
//     if (logicval_to_str(Old_) != logicval_to_str(New_)) {
//         inconsistent = true;
//         printf("    Inconsistency: %s output is %s but input suggests %s or %s \n", nodetype_to_str(node->type), logicval_to_str(Old), logicval_to_str(New), logicval_to_str(New_));    }
//     return updated;
// }


// bool justify_or(NSTRUC *node) {
//     bool updated = false;
//     bool has_one = false;
//     LogicVal Old;
//     LogicVal Old_;
//     LogicVal New;
//     LogicVal New_;
//     if (node->d_value == L_DB || node->d_value == L_0) {
//         for (int j = 0; j < node->fin; ++j) {
//             NSTRUC *input = node->unodes[j];
//             if (node->d_value == L_0 && (input->d_value == L_D || input->d_value == L_DB || input->d_value == L_1)) {
//                 inconsistent = true;
//                 printf("    Inconsistency: %s output is %s but input is %s\n", nodetype_to_str(node->type), logicval_to_str(node->d_value), logicval_to_str(input->d_value));
//                 updated = false;
//                 return updated;
//             }
//         }
//         for (int j = 0; j < node->fin; ++j) {
//             NSTRUC *input = node->unodes[j];
//             if (input->d_value == L_X) {
//                 input->d_value = L_0;
//                 printf("        Justified node %d to %s for node %d = %s (%s)\n", input->num, logicval_to_str(L_0), node->num, logicval_to_str(node->d_value), nodetype_to_str(node->type));
//                 updated = true;
//                 updated |= justify_input(input);
//                 if (inconsistent) {
//                     input->d_value = d_not(input->d_value);
//                     printf("    Backtracking on node %d (reset to %s)\n", input->num, logicval_to_str(input->d_value));
//                     return updated;
//                 } else {
//                     continue;
//                 }
//             }
//             if (input->d_value == L_1) {
//                 inconsistent = true;
//                 printf("    Inconsistency: %s output is %s but input is %s\n", nodetype_to_str(node->type), logicval_to_str(node->d_value), logicval_to_str(input->d_value));
//             }
//         }
//     } else if (node->d_value == L_1 || node->d_value == L_D) {
//         for (int j = 0; j < node->fin; ++j) {
//             NSTRUC *input = node->unodes[j];
//             if (input->d_value == L_X && !has_one) {
//                 input->d_value = L_1;
//                 printf("        Justified node %d to 1 for node %d = %s (%s)\n",
//                        input->num, node->num, logicval_to_str(node->d_value), nodetype_to_str(node->type));
//                 updated = true;
//                 updated |= justify_input(input);
//                 if (inconsistent) {
//                     input->d_value = d_not(input->d_value);
//                     inconsistent = false;
//                     has_one = false;
//                     printf("    Backtracking on node %d (reset to %s)\n", input->num, logicval_to_str(input->d_value));
//                     continue;
//                 } else {
//                     has_one = true;
//                     continue;
//                 }
//             } else if (input->d_value == L_1) {
//                 has_one = true;
//                 continue;
//             }
//         }
//         if (!has_one) {
//             inconsistent = true;
//             printf("    Inconsistency: %s output is 1 but no input is 1\n", nodetype_to_str(node->type));
//         }
//     }
//     node->force_d_value = false;
//     New = evaluate_d_gate(node);
//     node->force_d_value = true;
//     Old = node->d_value;
//     Old_ = Old;
//     New_ = New;
//     if (Old == L_D || Old == L_DB) {
//         if (New_ == L_D) {
//             New_ = L_1;
//         } else if (New_ == L_DB) {
//             New_ = L_0;
//         }
//         if (Old == L_D) {
//             Old_ = L_1;
//         } else if (Old == L_DB) {
//             Old_ = L_0;
//         }
//     }
//     if (logicval_to_str(Old_) != logicval_to_str(New_)) {
//         inconsistent = true;
//         printf("    Inconsistency: %s output is %s but input suggests %s or %s\n", nodetype_to_str(node->type), logicval_to_str(Old), logicval_to_str(New), logicval_to_str(New_));    
//     }
//     return updated;
// }


// bool justify_nor(NSTRUC *node) {
//     bool updated = false;
//     bool has_one = false;
//     LogicVal Old;
//     LogicVal Old_;
//     LogicVal New;
//     LogicVal New_;

//     if (node->d_value == L_D || node->d_value == L_1) {
//         for (int j = 0; j < node->fin; ++j) {
//             NSTRUC *input = node->unodes[j];
//             if (node->d_value == L_1 && (input->d_value == L_D || input->d_value == L_DB || input->d_value == L_1)) {
//                 inconsistent = true;
//                 printf("    Inconsistency: %s output is %s but input is %s\n", nodetype_to_str(node->type), logicval_to_str(node->d_value), logicval_to_str(input->d_value));
//                 updated = false;
//                 return updated;
//             }
//         }
//         for (int j = 0; j < node->fin; ++j) {
//             NSTRUC *input = node->unodes[j];
//             if (input->d_value == L_X) {
//                 input->d_value = L_0;
//                 printf("        Justified node %d to %s for node %d = %s (%s)\n", input->num, logicval_to_str(L_0), node->num, logicval_to_str(node->d_value), nodetype_to_str(node->type));
//                 updated = true;
//                 updated |= justify_input(input);
//                 if (inconsistent) {
//                     input->d_value = d_not(input->d_value);
//                     printf("    Backtracking on node %d (reset to %s)\n", input->num, logicval_to_str(input->d_value));
//                     return updated;
//                 } else {
//                     continue;
//                 }
//             }
//             if (input->d_value == L_1) {
//                 inconsistent = true;
//                 printf("    Inconsistency: %s output is %s but input is %s\n", nodetype_to_str(node->type), logicval_to_str(node->d_value), logicval_to_str(input->d_value));
//             }
//         }
//     } else if (node->d_value == L_0 || node->d_value == L_DB) {
//         for (int j = 0; j < node->fin; ++j) {
//             NSTRUC *input = node->unodes[j];
//             if (input->d_value == L_X && !has_one) {
//                 input->d_value = L_1;
//                 printf("        Justified node %d to 1 for node %d = %s (%s)\n",
//                        input->num, node->num, logicval_to_str(node->d_value), nodetype_to_str(node->type));
//                 updated = true;
//                 updated |= justify_input(input);
//                 if (inconsistent) {
//                     input->d_value = d_not(input->d_value);
//                     inconsistent = false;
//                     has_one = false;
//                     printf("    Backtracking on node %d (reset to %s)\n", input->num, logicval_to_str(input->d_value));
//                     continue;
//                 } else {
//                     has_one = true;
//                     continue;
//                 }
//             } else if (input->d_value == L_1) {
//                 has_one = true;
//                 continue;
//             }
//         }
//         if (!has_one) {
//             inconsistent = true;
//             printf("    Inconsistency: %s output is 0 but no input is 1\n", nodetype_to_str(node->type));
//         }
//     }
//     node->force_d_value = false;
//     New = evaluate_d_gate(node);
//     node->force_d_value = true;
//     Old = node->d_value;
//     Old_ = Old;
//     New_ = New;
//     if (Old == L_D || Old == L_DB) {
//         if (New_ == L_D) {
//             New_ = L_1;
//         } else if (New_ == L_DB) {
//             New_ = L_0;
//         }
//         if (Old == L_D) {
//             Old_ = L_1;
//         } else if (Old == L_DB) {
//             Old_ = L_0;
//         }
//     }
//     if (logicval_to_str(Old_) != logicval_to_str(New_)) {
//         inconsistent = true;
//         printf("    Inconsistency: %s output is %s but input suggests %s or %s\n", nodetype_to_str(node->type), logicval_to_str(Old), logicval_to_str(New), logicval_to_str(New_));    }
//     return updated;
// }

// bool justify_not(NSTRUC *node) {
//     bool updated = false;
//     LogicVal expected;
//     if (node->d_value == L_DB || node->d_value == L_D || node->d_value == L_1 || node->d_value == L_0) {
//         if (node->d_value == L_D || node->d_value == L_1) {
//             expected = L_0;
//         } else if (node->d_value == L_DB || node->d_value == L_0) {
//             expected = L_1;
//         }
//         for (int j = 0; j < node->fin; ++j) {
//             NSTRUC *input = node->unodes[j];
//             if (input->d_value == L_X) {
//                 input->d_value = expected;
//                 printf("        Justified node %d to %s for node %d = %s (%s)\n", input->num, logicval_to_str(expected), node->num, logicval_to_str(node->d_value), nodetype_to_str(node->type));
//                 updated |= justify_input(input);
//                 if (inconsistent) {
//                     input->d_value = d_not(input->d_value);
//                     printf("    Backtracking on node %d (reset to %s)\n", input->num, logicval_to_str(input->d_value));
//                     return updated;
//                 } else {
//                     continue;
//                 }
//             } else if (input->d_value != expected) {
//                 inconsistent = true;
//                 printf("    Inconsistency at node %d: expected %s, found %s (%s)\n",
//                        input->num, logicval_to_str(expected), logicval_to_str(input->d_value), nodetype_to_str(node->type));
//                 return updated;
//             }
//         }

//     }
//     return updated;
// }

// bool justify_xor(NSTRUC *node) {
//     bool updated = false;
//     int num_unknown = 0;
//     LogicVal known_val = L_X;
//     bool has_known = false;

//     for (int i = 0; i < node->fin; ++i) {
//         if (node->unodes[i]->d_value != L_X) {
//             if (!has_known) {
//                 known_val = node->unodes[i]->d_value;
//                 has_known = true;
//             }
//         } else {
//             num_unknown++;
//         }
//     }

//     if (node->d_value == L_1 || node->d_value == L_D) {
//         // Need odd number of 1's among inputs
//         for (int i = 0; i < node->fin; ++i) {
//             NSTRUC *input = node->unodes[i];
//             if (input->d_value == L_X) {
//                 input->d_value = (known_val == L_1 || known_val == L_D) ? L_0 : L_1;
//                 printf("        Justified XOR input node %d to %s for output %s\n", input->num, logicval_to_str(input->d_value), logicval_to_str(node->d_value));
//                 updated = true;
//                 updated |= justify_input(input);
//                 if (inconsistent) {
//                     input->d_value = d_not(input->d_value);
//                     return updated;
//                 }
//                 break; // Only justify one at a time
//             }
//         }
//     } else if (node->d_value == L_0 || node->d_value == L_DB) {
//         // Need even number of 1's among inputs
//         for (int i = 0; i < node->fin; ++i) {
//             NSTRUC *input = node->unodes[i];
//             if (input->d_value == L_X) {
//                 input->d_value = (known_val == L_1 || known_val == L_D) ? L_1 : L_0;
//                 printf("        Justified XOR input node %d to %s for output %s\n", input->num, logicval_to_str(input->d_value), logicval_to_str(node->d_value));
//                 updated = true;
//                 updated |= justify_input(input);
//                 if (inconsistent) {
//                     input->d_value = d_not(input->d_value);
//                     return updated;
//                 }
//                 break;
//             }
//         }
//     }
//     node->force_d_value = false;
//     LogicVal New = evaluate_d_gate(node);
//     node->force_d_value = true;
//     LogicVal Old = node->d_value;
//     LogicVal Old_ = Old;
//     LogicVal New_ = New;
//     if (Old == L_D || Old == L_DB) {
//         if (New_ == L_D) {
//             New_ = L_1;
//         } else if (New_ == L_DB) {
//             New_ = L_0;
//         }
//         if (Old == L_D) {
//             Old_ = L_1;
//         } else if (Old == L_DB) {
//             Old_ = L_0;
//         }
//     }
//     if (logicval_to_str(Old_) != logicval_to_str(New_)) {
//         inconsistent = true;
//         printf("    Inconsistency: %s output is %s but input suggests %s or %s\n",
//             nodetype_to_str(node->type), logicval_to_str(Old), logicval_to_str(New), logicval_to_str(New_));
//     }
//     return updated;
// }

// bool justify_xnor(NSTRUC *node) {
//     bool updated = false;
//     int num_unknown = 0;
//     LogicVal known_val = L_X;
//     bool has_known = false;

//     for (int i = 0; i < node->fin; ++i) {
//         if (node->unodes[i]->d_value != L_X) {
//             if (!has_known) {
//                 known_val = node->unodes[i]->d_value;
//                 has_known = true;
//             }
//         } else {
//             num_unknown++;
//         }
//     }

//     if (node->d_value == L_1 || node->d_value == L_D) {
//         // Need inputs same
//         for (int i = 0; i < node->fin; ++i) {
//             NSTRUC *input = node->unodes[i];
//             if (input->d_value == L_X) {
//                 input->d_value = (known_val == L_1 || known_val == L_D) ? L_1 : L_0;
//                 printf("        Justified XNOR input node %d to %s for output %s\n", input->num, logicval_to_str(input->d_value), logicval_to_str(node->d_value));
//                 updated = true;
//                 updated |= justify_input(input);
//                 if (inconsistent) {
//                     input->d_value = d_not(input->d_value);
//                     return updated;
//                 }
//                 break;
//             }
//         }
//     } else if (node->d_value == L_0 || node->d_value == L_DB) {
//         // Need inputs different
//         for (int i = 0; i < node->fin; ++i) {
//             NSTRUC *input = node->unodes[i];
//             if (input->d_value == L_X) {
//                 input->d_value = (known_val == L_1 || known_val == L_D) ? L_0 : L_1;
//                 printf("        Justified XNOR input node %d to %s for output %s\n", input->num, logicval_to_str(input->d_value), logicval_to_str(node->d_value));
//                 updated = true;
//                 updated |= justify_input(input);
//                 if (inconsistent) {
//                     input->d_value = d_not(input->d_value);
//                     return updated;
//                 }
//                 break;
//             }
//         }
//     }
//     node->force_d_value = false;
//     LogicVal New = evaluate_d_gate(node);
//     node->force_d_value = true;
//     LogicVal Old = node->d_value;
//     LogicVal Old_ = Old;
//     LogicVal New_ = New;
//     if (Old == L_D || Old == L_DB) {
//         if (New_ == L_D) {
//             New_ = L_1;
//         } else if (New_ == L_DB) {
//             New_ = L_0;
//         }
//         if (Old == L_D) {
//             Old_ = L_1;
//         } else if (Old == L_DB) {
//             Old_ = L_0;
//         }
//     }
//     if (logicval_to_str(Old_) != logicval_to_str(New_)) {
//         inconsistent = true;
//         printf("    Inconsistency: %s output is %s but input suggests %s or %s\n",
//             nodetype_to_str(node->type), logicval_to_str(Old), logicval_to_str(New), logicval_to_str(New_));
//     }
//     return updated;
// }




// bool justify_input(NSTRUC *node) {
//     switch (node->type) {
//         case BRCH:
//         case BUFFER:
//             return justify_branch(node);
//         case AND:
//             return justify_and(node);
//         case OR:
//             return justify_or(node);
//         case NAND:
//             return justify_nand(node);
//         case NOR:
//             return justify_nor(node);
//         case NOT:
//             return justify_not(node);
//         case XOR:
//             return justify_xor(node);
//         case XNOR:
//             return justify_xnor(node);
//         default:
//             return false;
//     }
// }

// bool already_justified[MAX_NODES] = {false};

// bool justify_test_pattern() {
//     bool any_update = false;
//     inconsistent = false;
//     printf("Starting full recursive justification...\n");
//     bool justified_this_pass[Nnodes];
//     memset(justified_this_pass, 0, sizeof(justified_this_pass));
//     bool updated = true;
//     while (updated) {
//         updated = false;
//         for (int i = 0; i < Nnodes; ++i) {
//             NSTRUC *node = &Node[i];
//             if (node->d_value != L_X) {
//                 if (justified_this_pass[i] || already_justified[i]) {
//                     continue;
//                 }
//                 if ((node->d_value == L_D || node->d_value == L_DB) && node->level != fault_node->level) {
//                     continue;
//                 }
//                 printf("    Justifying node %d with value %s\n", node->num, logicval_to_str(node->d_value));            
//                 inconsistent = false;
//                 if (justify_input(node)) {
//                     justified_this_pass[i] = true;
//                     already_justified[i] = true;
//                     updated = true;
//                 }
//                 if (inconsistent) {
//                     printf("    Inconsistency detected in justification of node %d\n", node->num);
//                     break;
//                 }
//             }
//         }
//         any_update |= updated;
//         // If no more updates and inconsistency was triggered, we exit
//         if (inconsistent) {
//             break;
//         }
//     }
//     if (!any_update) {
//         printf("No updates were made during justification.\n");
//     }
//     if (!inconsistent) {
//         printf("Justification complete with no inconsistencies.\n");
//         printf("Final Primary Input values after justification:\n");
//         for (int i = 0; i < Npi; ++i) {
//             printf("  PI %d (Node %d): %s\n", i, Pinput[i]->num, logicval_to_str(Pinput[i]->d_value));
//         }
//         return 0;
//     }
//     return any_update && !inconsistent;
// }

// void dalg() {
//     // fault_reached_output = false;
//     dfrontier_top = -1;
//     for (int i = 0; i < Nnodes; ++i) {
//         tried_d_frontier[i] = false;
//     }
//     reset_d_values();
//     char saved_cp[MAXLINE];
//     strncpy(saved_cp, cp, MAXLINE);
//     char dummy_cp[] = "temp_scoap.txt";
//     cp = dummy_cp;
//     scoap();
//     cp = saved_cp;

//     int fault_num = -1, fault_val = -1;
//     char output_file[MAXLINE];
//     if (sscanf(cp, "%d %d %s", &fault_num, &fault_val, output_file) != 3) {
//         printf("Usage: DLAG <node> <0|1> <output_file>\n");
//         return;
//     }

//     FILE *fp = fopen(output_file, "w");
//     if (!fp) {
//         printf("Cannot open output file: %s\n", output_file);
//         return;
//     }

//     bool found_d = false;
//     bool justification_success = false;
//     bool attempted = false;
//     fprintf(fp, "\n");

//     while (!justification_success && !attempted) {
//         attempted = true;

//         reset_d_values();
//         if (!inject_fault(fault_num, fault_val)) {
//             printf("Invalid fault value: %d\n", fault_val);
//             fclose(fp);
//             return;
//         }

//         // Forward implication
//         for (int level = 0; level <= get_max_level(); ++level) {
//             for (int i = 0; i < Nnodes; ++i) {
//                 if (Node[i].level == level && Node[i].fin > 0) {
//                     Node[i].d_value = evaluate_d_gate(&Node[i]);
//                 }
//             }
//         }
        
//         bool updated = true;
//         bool finished = false;
//         while (updated && !finished) {
//             updated = false;   

//             int target_nodes[MAX_NODES];
//             LogicVal fanout_values[MAX_NODES];
//             int target_node = -1;
            
//             // Case 1: No D-frontier nodes on the stack, find one
//             if (dfrontier_top == -1) {
//                 for (int i = 0; i < Nnodes; ++i) {
//                     if ((Node[i].d_value == L_D || Node[i].d_value == L_DB) &&
//                         !Node[i].visited && !tried_d_frontier[i]) {
//                         target_node = i;
//                         already_justified[i] = false;
//                         printf("[D-Frontier] Found new target node: %d (d_value = %s)\n",
//                                Node[i].num, logicval_to_str(Node[i].d_value));
//                         break;
//                     }
//                 }
            
//                 if (target_node == -1) {
//                     break; // Nothing to do
//                 }
            
//                 target_nodes[0] = target_node;
//                 fanout_values[0] = Node[target_node].d_value;
            
//                 for (int i = 0; i < Nnodes; ++i) {
//                     already_justified[i] = false;
//                 }
            
//                 tried_d_frontier[target_node] = true;
//                 propagate_fault(target_node);
//             } else {
//                 if (!pop_dfrontier(target_nodes, fanout_values)) {
//                     printf("[D-Frontier] Stack empty, no more fanout nodes to try. Exiting loop.\n");
//                     break;
//                 }
            
//                 for (int i = 0; i < Nnodes; ++i) {
//                     already_justified[i] = false;
//                 }
            
//                 for (int i = 0; i < last_popped_count; ++i) {
//                     int node_num = target_nodes[i];
//                     int node_index = -1;
//                     for (int j = 0; j < Nnodes; ++j) {
//                         if (Node[j].num == node_num) {
//                             node_index = j;
//                             break;
//                         }
//                     }
//                     Node[node_index].d_value = fanout_values[i];
//                     tried_d_frontier[node_index] = true;
//                     propagate_fault(node_index);
//                 }
//             }
            

//             fault_reached_output = false;
//             updated = true;

//             // Forward imply again
//             for (int level = 0; level <= get_max_level(); ++level) {
//                 for (int j = 0; j < Nnodes; ++j) {
//                     if (Node[j].level == level && Node[j].fin > 0 && !Node[j].force_d_value) {
//                         Node[j].d_value = evaluate_d_gate(&Node[j]);
//                     }
//                 }
//             }

//             // See if fault reached any PO
//             found_d = false;
//             for (int k = 0; k < Npo; ++k) {
//                 LogicVal v = Poutput[k]->d_value;
//                 if (v == L_D || v == L_DB) {
//                     found_d = true;
//                     break;
//                 }
//             }

//             if (found_d) {
//                 while (justify_test_pattern()) { }
//                 if (!inconsistent) {
//                     for (int pi = 0; pi < Npi; ++pi) {
//                         switch (Pinput[pi]->d_value) {
//                             case L_0: fprintf(fp, "0"); break;
//                             case L_D: fprintf(fp, "1"); break;
//                             case L_1: fprintf(fp, "1"); break;
//                             case L_DB: fprintf(fp, "0"); break;
//                             default:  fprintf(fp, "x"); break;
//                         }
//                         if (pi < Npi - 1) fprintf(fp, ",");
//                     }
//                     fprintf(fp, "\n");
//                     fflush(fp);
//                     // reset_d_values();
//                     inject_fault(fault_num, fault_val);
//                     printf("DALG: Test pattern generated successfully.\n");
//                     printf("Writing '");
//                     for (int pi = 0; pi < Npi; ++pi) {
//                         switch (Pinput[pi]->d_value) {
//                             case L_0: printf("0"); break;
//                             case L_D: printf("1"); break;
//                             case L_1: printf("1"); break;
//                             case L_DB: printf("0"); break;
//                             default: printf("x"); break;
//                         }
//                         if (pi < Npi - 1) printf(",");
//                     }
//                     printf("' to file %s\n", output_file);
//                     reset_d_values();
//                     inject_fault(fault_num, fault_val);
//                     printf("DALG: Test pattern generated successfully.\n");
//                 } else {
//                     // pop_state();
//                     printf("Trying next D-frontier node.\n");
//                     inconsistent = false;
//                 }
//             }
//             if (dfrontier_top == -1) {
//                 inconsistent = true;  // D-frontier is empty, so we cannot generate a valid test pattern
//                 finished = true;
//                 printf("D-frontier is empty. No further nodes to explore. Keeping inconsistency true.\n");
//             }
//         }
//     }
//     fclose(fp);
//     printf("DALG Completed.\n");
// }
        

// // READ ../auto-tests-phase3/ckts/c17.ckt
// // DALG 20 0 ../auto-tests-phase3/outputs/podem/c17_podem_20_0.out
// // QUIT