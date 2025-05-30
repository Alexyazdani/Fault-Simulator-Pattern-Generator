/*
Alexander Yazdani
USC Spring 2025
EE658
*/

#include "logicsim.h"

/*-----------------------------------------------------------------------
input: nothing
output: nothing
called by: main
description:
    Simulates logic gate evaluation.
-----------------------------------------------------------------------*/
int evaluate_gate(NSTRUC *node) {
    if (node == nullptr) {
        printf("ERROR: Received NULL node in evaluate_gate!\n");
        exit(1);
    }
    
    // for (int i = 0; i < node->fin; i++) {
    //     if (node->unodes[i] == nullptr) {
    //         printf("ERROR: Node %d input %d (unodes[%d]) is NULL!\n", node->num, i, i);
    //         exit(1);
    //     }
    // }
    // printf("%d", node->type);
    if (node->type == IPT) {
        // printf("PI Node %d: Value = %d\n", node->num, node->value);
        return node->value;
    }
    if (node->fin == 0) return -1;
    if (node->type == BRCH) {
        int branch_value = node->unodes[0]->value;
        // printf("Branch Node %d: Propagating %d from %d\n", 
        //     node->num, branch_value, node->unodes[0]->num);
        return branch_value;
    }
    
    int result = node->unodes[0]->value;

    // Default values for NOT and BUFFER
    if (node->type == NOT) return (result == -1) ? -1 : !result;
    if (node->type == BUFFER) return result;

    // Iterate through all inputs and evaluate the gate
    for (int i = 1; i < node->fin; i++) {
        int input_val = node->unodes[i]->value; 
        // printf("Node %d: Inputs = %d (from %d), %d (from %d)\n",
        //     node->num, node->unodes[0]->value, node->unodes[0]->num,
        //     node->unodes[1]->value, node->unodes[1]->num);
        switch (node->type) {
            case AND: 
                if (result == 0 || input_val == 0) {
                    // printf("Node %d: Output = %d\n", node->num, 0); 
                    return 0;
                }
                else if (result == -1 || input_val == -1) 
                    result = -1;
                break;
            case OR: 
                if (result == 1 || input_val == 1) {
                    // printf("Node %d: Output = %d\n", node->num, 1); 
                    return 1;
                }
                else if (result == -1 || input_val == -1) 
                    result = -1;
                break;
            case XOR: 
                if (result == -1 || input_val == -1) 
                    result = -1;
                else 
                    result = result ^ input_val;
                break;
            case NAND: 
                if (result == 0 || input_val == 0) {
                    // printf("Node %d: Output = %d\n", node->num, 1); 
                    return 1;
                }
                else if (result == -1 || input_val == -1) 
                    result = -1;
                break; 
            case NOR: 
                if (result == 1 || input_val == 1) {
                    // printf("Node %d: Output = %d\n", node->num, 0); 
                    return 0;
                }
                else if (result == -1 || input_val == -1) 
                    result = -1;
                break;
            case XNOR: 
                if (result == -1 || input_val == -1) 
                    result = -1;
                else 
                    result = result ^ input_val;
                break;
            case BUFFER: break;
            case NOT: break;
            case IPT: break;
            case BRCH: break;
        }
    }
    
    // Invert the result for NAND, NOR, XNOR gate but only if result is not unknown
    if ((node->type == NAND || node->type == NOR || node->type == XNOR) && result != -1) 
        result = !result;
    // printf("Node %d: Output = %d\n", node->num, result);
    return result;
}


void logicsim() {
    
    char input_file[MAXLINE], output_file[MAXLINE];

    // Read I/O file names from user input
    sscanf(cp, "%s %s", input_file, output_file);

    // Open the input and output files
    FILE *inFile = fopen(input_file, "r");
    FILE *outFile = fopen(output_file, "w");

    for (int i = 0; i < Npo; i++) {
        if (!Poutput[i]) {
            printf("ERROR: Poutput[%d] is NULL!\n", i);
        }
        fprintf(outFile, "%d", Poutput[i]->num);    // Print PO node ID
        if (i < Npo - 1) fprintf(outFile, ",");     // Comma-separated if not last
    }
    fprintf(outFile, "\n");

    // Read input values and assign them to corresponding primary input nodes
    char line[MAXLINE];
    if (inFile == NULL) {
        printf("ERROR: Input file is NULL!\n");
        exit(1);
    }
    char lev_output_file[] = "temp_lev.txt";
    char *original_cp = cp;  // Save the original cp value
    cp = lev_output_file;     // Set cp to the levelization filename
    lev();                    // Call levelization
    cp = original_cp;         // Restore cp to its original value
    
    
    fgets(line, MAXLINE, inFile);

    while (fgets(line, MAXLINE, inFile)) {
        // printf("Read line: %s", line);

        int values[MAXLINE], val_count = 0;

        char *token = strtok(line, ",");

        while (token && val_count < Npi) {
            size_t len = strlen(token);
            if (token[len - 1] == '\n') {
                token[len - 1] = '\0';  // Remove the newline character
            }
            // printf("Parsed token: '%s' -> Assigned value: ", token);
            if (strcmp(token, "x") == 0) {
                values[val_count++] = -1;  // Assign -1 for 'x' (unknown)
                // printf("-1 (X)\n");
            } else {
                values[val_count++] = atoi(token);  // Convert numeric values to integers
                // printf("%d\n", values[val_count - 1]);
            }
            token = strtok(NULL, ",");
        }

        // Assign values to primary input nodes
        for (int i = 0; i < Npi; i++) {
            Pinput[i]->value = values[i];       
        }

        // Evaluate all gates level by level
        for (int level = 0; level <= Nnodes; level++) {
            // printf("we got here!");
            // fflush(stdout);
            for (int i = 0; i < Nnodes; i++) {
                if (Node[i].level == level && Node[i].ntype != PI) {
                    Node[i].value = evaluate_gate(&Node[i]);
                }
            }
        }
        printf("Final Output Values BEFORE Writing to File:\n");
        for (int i = 0; i < Npo; i++) {
            if (Poutput[i]) {
                printf("Poutput[%d]: Num=%d, Value=%d\n", i, Poutput[i]->num, Poutput[i]->value);
            } else {
                printf("ERROR: Poutput[%d] is NULL!\n", i);
            }
        }
        printf("\n");
        fflush(stdout);

        for (int i = 0; i < Npo; i++) {
            if (!Poutput[i]) {
                printf("ERROR: Poutput[%d] is NULL!\n", i);
                
            }

            if (Poutput[i]->value == -1) {
                fprintf(outFile, "x");
            } else {
                fprintf(outFile, "%d", Poutput[i]->value);  // Print PO node value
            }
            if (i < Npo - 1) fprintf(outFile, ",");  // Comma-separated if not last
        }
        fprintf(outFile, "\n"); // Newline after each test case

    }

    // Close files
    fclose(inFile);
    fclose(outFile);
}


 // read ../auto-tests-phase2/ckts/c499.ckt
 // lev lev_out.txt
 // logicsim ../auto-tests-phase2/inputs/logicsim/c499_b_tp25.tp c499_out.txt