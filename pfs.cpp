#include "pfs.h"

void pfs() {
    char lev_output_file[] = "temp_lev.txt";
    char *original_cp = cp;  // Save the original cp value
    cp = lev_output_file;     // Set cp to the levelization filename
    lev();                    // Call levelization
    cp = original_cp;         // Restore cp to its original value
    
    NSTRUC *np;
    char inputbuf1[MAXLINE], inputbuf2[MAXLINE], outputbuf[MAXLINE];
    sscanf(cp, "%s %s %s", inputbuf1, inputbuf2, outputbuf);	 
    ifstream input_file1;
    input_file1.open(inputbuf1);
    bool c_flag = false;
    int n;
    string input_line, flag;
    vector<vector<int> > inputpattern;
    vector<int> inputpatternline;
    ofstream output_file;
    output_file.open(outputbuf); 
    if (input_file1.is_open()) {
        while ( input_file1 ) {
            getline (input_file1, input_line);  
            stringstream X(input_line);
            while (getline(X, flag, ',')) {
                inputpatternline.push_back(stoi(flag));
            }
            inputpattern.push_back(inputpatternline);
            inputpatternline.clear();
        }
        input_file1.close();
    }
    else {
        cout << "Couldn't open file\n";
    }
    int LoP =  sizeof(int) * 8;
    std::set<std::string> unique_faults;
    for (int k = 1; k < inputpattern.size()-1; k++) {  //iterating over input patterns
        int i=0;
        n = 0;
        for (int node = 0;node<Nnodes; node++){
            np = &Node[node];
            np->ormask = 0;
            np->andmask = -1;
        }
        for (int j = 0; j < Nnodes; j++){    // iterate over all the nodes
            if (Node[j].num == inputpattern[0][n]) {
                np = &Node[j];	               
                Node[j].node_value = inputpattern[k][n];   
                if (inputpattern[k][n] == 0)
                    Node[j].node_value = 0;
                else if (inputpattern[k][n] == 1)
                    Node[j].node_value = -1;

                n = n + 1;         
                
            }
        }

        parallel_node_value(); // LOGICSIM
        //logicsim();

        for (int node=0; node<Nnodes; node++){
            np = &Node[node];
            np->true_value = np->node_value;
        }

        ifstream input_file2;
        input_file2.open(inputbuf2);
        if (input_file2.is_open()) {
            // Reset the file position at the beginning for each pattern
            input_file2.clear();  // ADDED: Clear any error flags
            input_file2.seekg(0); // ADDED: Move to beginning of file
            
            while(input_file2.peek() != EOF){
                int boundary = 0;
                int* fault_id = new int[LoP];
                int* fault_val = new int[LoP];
                
                // Reset nodes for new batch
                for (int node = 0;node<Nnodes; node++){
                    np = &Node[node];
                    np->node_value = np->true_value;  // ADDED: Reset to correct value
                    np->ormask = 0;
                    np->andmask = -1;
                }
                
                for(int fault_cnt = 0; fault_cnt < LoP; fault_cnt++){
                    getline (input_file2, input_line);  
                    stringstream X(input_line);
                    getline(X, flag, '@');
                    for(int i = 0; i < Nnodes; i++){
                        np = &Node[i];
                        if(np->num == stoi(flag)){
                            break;
                        }
                    }
                    fault_id[fault_cnt] = np->num;
                    getline(X, flag, '@');
                    if(stoi(flag) == 0){
                        np->andmask =  ~((~(np->andmask)) | (1<<fault_cnt));
                        np->ormask =   ~((~(np->ormask)) | (1<<fault_cnt));
                        fault_val[fault_cnt] = 0;

                    }else{
                        np->ormask =   ((np->ormask) | (1<<fault_cnt));
                        fault_val[fault_cnt] = 1;
                                    
                    }
                    boundary++;
                    if(input_file2.peek() == EOF){
                        break;  // CHANGED: Don't close file yet
                    } 
                }

                parallel_node_value();

                for (int x = 0; x < Npo; x++) {
                    if (Poutput[x]->true_value != Poutput[x]->node_value) {
                        int cmp_res = Poutput[x]->true_value ^ Poutput[x]->node_value;
                        for (int fcnt = 0; fcnt < boundary; fcnt++) {
                            if ((cmp_res >> fcnt) & 1) {  // CHANGED: Direct bit checking
                                std::string fault_str = std::to_string(fault_id[fcnt]) + "@" + std::to_string(fault_val[fcnt]);
                                unique_faults.insert(fault_str);
                                if (fault_id[fcnt] == 173) {
                                    // printf("Fault ID: %d\n", fault_id[fcnt]);
                                    // printf("Fault Val: %d\n", fault_val[fcnt]);
                                    // printf("True Value: %d\n", Poutput[x]->true_value);
                                    // printf("Node Value: %d\n", Poutput[x]->node_value);
                                    // printf("cmp_res: %d\n", cmp_res);
                                }
                            }
                        }
                    }
                }
                
                delete[] fault_id;
                delete[] fault_val;
            }
            input_file2.close();  // MOVED: Close file after all batches
        }
        else {
            cout << "Couldn't open file\n";
        }
    }

    // printf("Unique Faults (Set Size: %lu):\n", unique_faults.size());
    for (const auto& fault : unique_faults) {
        output_file << fault << "\n";  // Now we write to file
    }
}
