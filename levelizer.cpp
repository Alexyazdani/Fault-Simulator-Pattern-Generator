/*
Alexander Yazdani
USC Spring 2025
EE658
*/

#include "levelizer.h"

/*-----------------------------------------------------------------------
input: nothing
output: nothing
called by: main
description:
    This routine levelizes the circuit.
-----------------------------------------------------------------------*/

int get_max_level() {
    int max_level = 0;
    for (int i = 0; i < Nnodes; ++i)
        if (Node[i].level > max_level)
            max_level = Node[i].level;
    return max_level;
}

void lev() {

    // Grab filename from user input
    char filename[MAXLINE];
    while (*cp == ' ' || *cp == '\t') cp++;
    if (sscanf(cp, "%s", filename) != 1) {
        printf("Missing filename!\n");
        return;
    }

    // Open the output file for writing
    FILE *outFile = fopen(filename, "w");
    for (int i = 0; i < Nnodes; i++) {
        // Initialize all levels to -1
        Node[i].level = -1;
    }
    for (int i = 0; i < Npi; i++) {
        // Set primary inputs to level 0
        Pinput[i]->level = 0;
    }

    // Loop to update levels of all nodes
    bool updated;
    do {
        // Flag to check if updates were made
        updated = false;
        for (int i = 0; i < Nnodes; i++) {
            if (Node[i].level != -1) continue;  // Skip PI nodes
            int max_level = -1;
            bool all_inputs_ready = true;
            // Iterate through all fanin nodes
            for (int j = 0; j < Node[i].fin; j++) {
                // Check if all fanin nodes have a level assigned
                if (Node[i].unodes[j]->level == -1) {
                    // Stop level calculation if fanin nodes are not ready
                    all_inputs_ready = false;
                    break;
                }
                // Set max_level to the highest level of all fanin nodes
                max_level = std::max(max_level, Node[i].unodes[j]->level);
            }
            if (all_inputs_ready) {
                // Set the lvel to be 1+ the max level of fanin nodes
                Node[i].level = max_level + 1;
                // Set updated flag
                updated = true;
            }
        }
    // Repeat loop until updates finish
    } while (updated);

    Nlevels = get_max_level() + 1;

    // Print the header:
    const char *base = strrchr(inp_name.c_str(), '/') ? strrchr(inp_name.c_str(), '/') + 1 : inp_name.c_str();
    const char *dot = strrchr(base, '.');
    fprintf(outFile, "%.*s\n", dot ? (int)(dot - base) : (int)strlen(base), base);
    fprintf(outFile, "#PI: %d\n", Npi);
    fprintf(outFile, "#PO: %d\n", Npo);
    fprintf(outFile, "#Nodes: %d\n", Nnodes);

    // Count and print the number of gates:
    int Ngates = 0;
    for (int i = 0; i < Nnodes; i++) {
        NSTRUC *np = &Node[i];  
        if (np->type > 1) {
            Ngates++;
        }
    }
    fprintf(outFile, "#Gates: %d\n", Ngates);

    // Print the level of each node:
    for (int i = 0; i < Nnodes; i++) {
        fprintf(outFile, "%d %d\n", Node[i].num, Node[i].level);
    }
    fclose(outFile);
}
