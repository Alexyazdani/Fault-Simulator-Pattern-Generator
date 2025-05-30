#include "dfs.h"


void dfs_single(const vector<int>& input_pattern, set<string>& detected_faults) {
    int n = 0;
    for (int node = 0; node < Nnodes; node++) {
        NSTRUC* np = &Node[node];
        np->ormask = 0;    // Reset the fault mask for each node
        np->andmask = 1;
        if (np->type == 0) {
            np->node_value = input_pattern[n];  // Assign the input values to PIs
            n++;
        }
    }

    // Run initial simulation
    parallel_node_value();

    for (int i = 0; i < Npo; i++) {
        NSTRUC* np_output = Poutput[i];
        np_output->true_value = np_output->node_value;  // Save correct output values from initial simulation
    }

    for (int i = 0; i < Nnodes; i++) {
        NSTRUC* np = &Node[i];

        // Save original value of nodes
        int original_value = np->node_value;
        
        if (np->node_value == 0) {  // Stuck-at-1 fault
            np->andmask = 1;
            np->ormask = 1;
        } else {                    // Stuck-at-0 fault
            np->andmask = 0;
            np->ormask = 0;
        }

     

        parallel_node_value();  // Run the simulation with the fault injected

        for (int j = 0; j < Npo; j++) {  // Loop through each primary output (PO)
            NSTRUC* np_output = Poutput[j];

            // Compare the output value with the correct value
            if (np_output->true_value != np_output->node_value) {
                string fault = to_string(np->num) + "@" + to_string(np->node_value);
                detected_faults.insert(fault);  // Store the detected fault
                // printf("Detected fault: %s\n", fault.c_str());
            }
        }
        
        // Restore the original value of the node
        np->node_value = original_value;
        np->andmask = 1;
        np->ormask = 0;
        // Run the simulation again with the original value
        parallel_node_value();
    }
}


void dfs() {
    char lev_output_file[] = "temp_lev.txt";
    char *original_cp = cp;  // Save the original cp value
    cp = lev_output_file;     // Set cp to the levelization filename
    lev();                    // Call levelization
    cp = original_cp;         // Restore cp to its original value
    
    // cout << "Running DFS..." << endl;

    NSTRUC *np;
    char inputbuf[MAXLINE], outputbuf[MAXLINE];
    sscanf(cp, "%s %s", inputbuf, outputbuf);  // Read input/output filenames


    ifstream input_file(inputbuf);
    ofstream output_file(outputbuf, ios::trunc);  // Clear previous output file

    if (!input_file.is_open()) {
        cout << "Error: Couldn't open input file" << endl;
        return;
    }
    if (!output_file.is_open()) {
        cout << "Error: Couldn't open output file" << endl;
        return;
    }

    vector<int> input_nodes;
    vector<int> input_pattern;
    set<string> detected_faults;  // Set to store detected faults
    string line;

    if (getline(input_file, line)) {
        stringstream ss(line);
        string token;
        while (getline(ss, token, ',')) {
            input_nodes.push_back(stoi(token));  // Store the node numbers
        }
    }

    // cout << "Input Nodes: ";
    // for (int node : input_nodes) {
    //     cout << node << " ";
    // }
    // cout << endl;
    while (getline(input_file, line)) {
        input_pattern.clear();  // Clear the previous pattern

        stringstream ss(line);
        string token;
        while (getline(ss, token, ',')) {
            input_pattern.push_back(stoi(token));  // Store the test pattern
        }
        // cout << "Test Pattern: ";
        // for (int value : input_pattern) {
        //     cout << value << " ";
        // }
        // cout << endl;
        for (size_t i = 0; i < input_nodes.size(); i++) {
            int node_num = input_nodes[i];
            for (int j = 0; j < Nnodes; j++) {
                if (Node[j].num == node_num) {
                    Node[j].value = input_pattern[i];  // Assign the value from the pattern
                    break;
                }
            }
        }
        detected_faults.clear();  // Clear previously detected faults
        dfs_single(input_pattern, detected_faults);  // Perform fault simulation
        // cout << "Detected Faults: ";
        // if (detected_faults.empty()) {
        //     cout << "No faults detected." << endl;
        // } else {
        //     for (const string& fault : detected_faults) {
        //         cout << fault << " ";
        //     }
        //     cout << endl;
        // }
        for (const string& fault : detected_faults) {
            output_file << fault << "\n";  // Write each fault to the output file
        }
        
    }
    input_file.close();  // Close the input file after processing all lines
    output_file.close();  // Close the output file
}
