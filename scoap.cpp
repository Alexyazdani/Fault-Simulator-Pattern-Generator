#include <cstdio>
#include <climits>
#include "scoap.h"
#include "levelizer.h"

extern NSTRUC *Node;
extern NSTRUC **Pinput, **Poutput;
extern int Nnodes, Npi, Npo;

int min_cc0(NSTRUC *np) {
    int min_val = INT_MAX;
    for (int i = 0; i < np->fin; ++i)
        if (np->unodes[i]->cc0 < min_val)
            min_val = np->unodes[i]->cc0;
    return min_val;
}

int min_cc1(NSTRUC *np) {
    int min_val = INT_MAX;
    for (int i = 0; i < np->fin; ++i)
        if (np->unodes[i]->cc1 < min_val)
            min_val = np->unodes[i]->cc1;
    return min_val;
}

int sum_cc0(NSTRUC *np) {
    int sum = 0;
    for (int i = 0; i < np->fin; ++i)
        sum += np->unodes[i]->cc0;
    return sum;
}

int sum_cc1(NSTRUC *np) {
    int sum = 0;
    for (int i = 0; i < np->fin; ++i)
        sum += np->unodes[i]->cc1;
    return sum;
}

void compute_cc(NSTRUC *np) {
    switch (np->type) {
        case 0: // PI
            np->cc0 = 1;
            np->cc1 = 1;
            break;
        case 1: // BRANCH
            np->cc0 = np->unodes[0]->cc0;
            np->cc1 = np->unodes[0]->cc1;
            break;
        case 9: // BUFFER
            np->cc0 = np->unodes[0]->cc0 + 1;
            np->cc1 = np->unodes[0]->cc1 + 1;
            break;
        case 5: // NOT
            np->cc0 = np->unodes[0]->cc1 + 1;
            np->cc1 = np->unodes[0]->cc0 + 1;
            break;
        case 3: // OR
            np->cc0 = sum_cc0(np) + 1;
            np->cc1 = min_cc1(np) + 1;
            break;
        case 4: // NOR
            np->cc1 = sum_cc0(np) + 1;
            np->cc0 = min_cc1(np) + 1;
            break;
        case 7: // AND
            np->cc0 = min_cc0(np) + 1;
            np->cc1 = sum_cc1(np) + 1;
            break;
        case 6: // NAND
            np->cc1 = min_cc0(np) + 1;
            np->cc0 = sum_cc1(np) + 1;
            break;
        case 2: // XOR
            np->cc0 = sum_cc0(np) + 1;
            np->cc1 = sum_cc1(np) + 1;
            break;
        case 8: // XNOR
            np->cc0 = sum_cc0(np) + 1;
            np->cc1 = sum_cc1(np) + 1;
            break;
        default:
            np->cc0 = np->cc1 = 100; // Unhandled Gate
            break;
    }
}

void compute_co() {
    // Default CO to large value for all nodes
    for (int i = 0; i < Nnodes; ++i)
        Node[i].co = INT_MAX;

    // Set CO for all POs to 0
    for (int i = 0; i < Npo; ++i)
        Poutput[i]->co = 0;

    // Back propagate CO
    for (int level = get_max_level(); level >= 0; --level) {
        for (int i = 0; i < Nnodes; ++i) {
            NSTRUC *np = &Node[i];
            if (np->level != level)
                continue;

            // Skip if no fanin (PI)
            if (np->fin == 0)
                continue;

            // Compute CO for each fanin
            for (int in_idx = 0; in_idx < np->fin; ++in_idx) {
                NSTRUC *fanin = np->unodes[in_idx];
                int co_through_gate;

                switch (np->type) {
                    case 7: // AND
                    case 3: // OR
                    {
                        int sum = 0;
                        for (int k = 0; k < np->fin; ++k) {
                            if (k == in_idx) continue;
                            if (np->type == 7)  // AND: need others to be 1
                                sum += np->unodes[k]->cc1;
                            else                // OR: need others to be 0
                                sum += np->unodes[k]->cc0;
                        }
                        co_through_gate = np->co + sum + 1;
                        break;
                    }

                    case 6: // NAND
                    case 4: // NOR
                    {
                        int sum = 0;
                        for (int k = 0; k < np->fin; ++k) {
                            if (k == in_idx) continue;
                            if (np->type == 6)  // NAND: like AND
                                sum += np->unodes[k]->cc1;
                            else               // NOR: like OR
                                sum += np->unodes[k]->cc0;
                        }
                        co_through_gate = np->co + sum + 1;
                        break;
                    }

                    case 2: // XOR
                    case 8: // XNOR
                    {
                        int sum = 0;
                        for (int k = 0; k < np->fin; ++k) {
                            if (k == in_idx) continue;
                            sum += std::min(np->unodes[k]->cc0, np->unodes[k]->cc1);
                        }
                        co_through_gate = np->co + sum + 1;
                        break;
                    }

                    case 5: // NOT
                    case 9: // BUFFER
                        co_through_gate = np->co + 1;
                        break;

                    case 1: // BRANCH
                        co_through_gate = np->co; // no +1
                        break;


                    default:
                        co_through_gate = INT_MAX;  // Unhandled Gate
                        break;
                }

                if (co_through_gate < fanin->co)
                    fanin->co = co_through_gate;
            }
        }
    }
}


void scoap() {
    // Levelize the circuit
    lev();
    char fname[MAXLINE];
    while (*cp == ' ' || *cp == '\t') cp++;
    if (sscanf(cp, "%s", fname) != 1) {
        printf("Missing output filename!\n");
        return;
    }

    FILE *fp = fopen(fname, "w");
    if (!fp) {
        printf("Cannot open output file %s\n", fname);
        return;
    }

    // Compute CC0 and CC1 in level order
    int max_level = get_max_level();
    for (int level = 0; level <= max_level; ++level) {
        for (int i = 0; i < Nnodes; ++i) {
            if (Node[i].level == level) {
                compute_cc(&Node[i]);
            }
        }
    }

    // Compute CO
    compute_co();

    // Write results
    for (int i = 0; i < Nnodes; ++i)
        fprintf(fp, "%d,%d,%d,%d\n", Node[i].num, Node[i].cc0, Node[i].cc1, Node[i].co);

    fclose(fp);
    // printf("SCOAP values written to %s\n", fname);
}


// READ ../auto-tests-phase3/ckts/c1.ckt
// SCOAP ../auto-tests-phase3/outputs/scoap/c1_scoap.out
// QUIT
