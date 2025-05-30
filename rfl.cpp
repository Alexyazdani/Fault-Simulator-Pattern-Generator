#include "rfl.h"
#include <stdio.h>
#include <string.h>

void rfl() {
    char output_file[MAXLINE];

    if (sscanf(cp, "%s", output_file) != 1) {
        printf("Error: Invalid command format. Use 'RFL <output-fl-file>'\n");
        return;
    }

    FILE *outFile = fopen(output_file, "w");
    if (!outFile) {
        printf("Error opening output file: %s\n", output_file);
        return;
    }

    // Iterate through all nodes in the circuit and generate stuck-at faults
    for (int i = 0; i < Nnodes; i++) {
        int node_id = Node[i].num;
        if (Node[i].type == IPT || Node[i].type == BRCH) {
            fprintf(outFile, "%d@0\n", node_id);  // Stuck-at 0
            fprintf(outFile, "%d@1\n", node_id);  // Stuck-at 1
        }
    }

    fclose(outFile);
}
