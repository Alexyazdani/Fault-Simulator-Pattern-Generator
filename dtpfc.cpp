#include "dtpfc.h"
#include "pfs.h"

void dtpfc() {
    char tp_file[MAXLINE], report_file[MAXLINE];
    int nTFCR;

    // Parse input arguments
    if (sscanf(cp, "%s %d %s", tp_file, &nTFCR, report_file) != 3) {
        printf("Error: Invalid command format. Use 'DTPFC <tp-fname> <freq> <tpfc-report-fname>'\n");
        return;
    }
    printf("[INFO] Parsed arguments: tp_file = %s, freq = %d, report_file = %s\n", tp_file, nTFCR, report_file);
    // Open input and output files
    ifstream tp_in(tp_file);
    ofstream report_out(report_file, ios::trunc);
    if (!tp_in.is_open() || !report_out.is_open()) {
        printf("Error: Couldn't open input or output files.\n");
        return;
    }






    // Generate pfs_fl.txt
    ofstream pfl("pfs_fl.txt");
    for (int i = 0; i < Nnodes; ++i) {
        pfl << Node[i].num << "@0\n";
        pfl << Node[i].num << "@1\n";
    }
    pfl.close();
    printf("[INFO] Generated pfs_fl.txt\n");








    // Total number of faults
    int total_fault_count = 2 * Nnodes;  // 2 stuck-at faults per node

    // Store detected faults
    set<string> detected_faults;
    int detected_count = 0;

    string line;
    int pattern_count = 0;

    while (getline(tp_in, line)) {
        vector<int> test_pattern;
        istringstream iss(line);
        string tok;

        // Read test pattern
        while (getline(iss, tok, ',')) {
            if (tok == "0") test_pattern.push_back(0);
            else if (tok == "1") test_pattern.push_back(1);
            else test_pattern.push_back(2); // treat 'x' or other chars as unknown
        }






        // Write pattern to temp_tp.txt
        ofstream tmp_tp("temp_tp.txt");
        for (int i = 0; i < Npi; ++i) {
            tmp_tp << Pinput[i]->num;
            if (i < Npi - 1) tmp_tp << ",";
        }
        tmp_tp << "\n";
        for (size_t j = 0; j < test_pattern.size(); ++j) {
            if (test_pattern[j] == 0) tmp_tp << "0";
            else if (test_pattern[j] == 1) tmp_tp << "1";
            else tmp_tp << "2";
            if (j < test_pattern.size() - 1) tmp_tp << ",";
        }
        tmp_tp << "\n";
        tmp_tp.close();

        // Set cp for PFS and run it
        char fs_cp[MAXLINE];
        snprintf(fs_cp, sizeof(fs_cp), "%s %s %s", "temp_tp.txt", "pfs_fl.txt", "pfs_out.txt");
        cp = fs_cp;
        pfs();

        // Read detected faults
        ifstream pfs_out("pfs_out.txt");
        string fault_line;
        while (getline(pfs_out, fault_line)) {
            detected_faults.insert(fault_line);
        }
        pfs_out.close();
        pattern_count++;






        // // Run fault simulation
        // dfs_single(test_pattern, detected_faults);
        // pattern_count++;

        // Update fault coverage every nTFCR patterns
        if (pattern_count % nTFCR == 0) {
            detected_count = detected_faults.size();
            double fc = (double(detected_count) / total_fault_count) * 100.0;
            report_out << fixed << setprecision(2) << fc << "\n";
        }
    }

    tp_in.close();
    report_out.close();
}


// READ ../ckts_all/c432.ckt
// LEV lev_c432.ckt
// TPG PODEM PFS my_patterns.tp -rtp v1 80
// TPFC 1000 10 patterns_tpfc.tp report_out.txt
// DTPFC my_patterns.tp 10 report_out.txt