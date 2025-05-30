#include "tpfc.h"

void tpfc() {
    char tp_file[MAXLINE], report_file[MAXLINE];
    int ntot, nTFCR;

    // Parse input argumentss
    if (sscanf(cp, "%d %d %s %s", &ntot, &nTFCR, tp_file, report_file) != 4) {
        printf("Error: Invalid command format. Use 'TPFC <tp-count> <freq> <output-tp-file> <report-file>'\n");
        return;
    }

    // Open output files
    ofstream tp_out(tp_file, ios::trunc);
    ofstream report_out(report_file, ios::trunc);
    if (!tp_out.is_open() || !report_out.is_open()) {
        printf("Error: Couldn't open output files.\n");
        return;
    }

    // Total number of faults
    int total_fault_count = 2 * Nnodes;  // 2 S@F per node

    // Initialize random seed
    srand(time(0));

    // Get the primary input nodes
    vector<int> input_nodes;
    for (int i = 0; i < Nnodes; i++) {
        if (Node[i].type == 0) {  // Primary Inputs (PI)
            input_nodes.push_back(Node[i].num);
        }
    }

    // Store detected faults in a set to avoid duplicates
    set<string> detected_faults;
    int detected_count = 0;

    // Generate `ntot` test patterns and run fault simulation
    for (int i = 0; i < ntot; i++) {
        vector<int> test_pattern;
        for (size_t j = 0; j < input_nodes.size(); j++) {
            test_pattern.push_back(rand() % 2); // Generate random 0 or 1
        }

        // Write test pattern to output file
        for (size_t j = 0; j < test_pattern.size(); j++) {
            tp_out << test_pattern[j];
            if (j < test_pattern.size() - 1) tp_out << ",";
        }
        tp_out << "\n";

        // Run fault simulation on the test pattern
        dfs_single(test_pattern, detected_faults);

        // Update fault coverage every `nTFCR` test patterns
        if ((i + 1) % nTFCR == 0) {
            detected_count = detected_faults.size();

            // Compute fault coverage
            double fc = (double(detected_count) / total_fault_count) * 100.0;

            // Write to output file
            report_out << fixed << setprecision(2) << fc << "\n";
        }
    }

    // Close output files
    tp_out.close();
    report_out.close();
}

// READ ../auto-tests-phase2/ckts/c880.ckt
// LEV lev_c880.txt
// TPFC 1000 10 tp_out.txt report_out.txt