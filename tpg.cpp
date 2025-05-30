// #include "circuit.h"
// #include "dalg.h"
// #include "podem.h"
// #include "dfs.h"
// #include "pfs.h"
// #include "tpg.h"
// #include "rfl.h"
// #include "rtpg.h"
// #include <vector>
// #include <fstream>
// #include <algorithm>
// #include <chrono>

// static bool use_podem = true;  // false = use DALG
// static bool use_dfs = true;    // false = use PFS

// // Generates the full fault list
// std::vector<std::pair<NSTRUC*, int>> generate_fault_list() {
//     std::vector<std::pair<NSTRUC*, int>> flist;
//     std::ofstream fout("fault_list.txt");
//     for (int i = 0; i < Nnodes; ++i) {
//         NSTRUC* np = &Node[i];
//         flist.emplace_back(np, 0);
//         flist.emplace_back(np, 1);
//         fout << np->num << "@0\n";
//         fout << np->num << "@1\n";
//     }
//     fout.close();
//     return flist;
// }

// extern char* cp;
// extern bool fault_reached_output;
// extern int Npi;
// extern NSTRUC** Pinput;


// bool run_atpg(NSTRUC* node, int sa_val, std::vector<int>& pattern, bool use_podem) {
//     static char temp_cp[128];
//     snprintf(temp_cp, sizeof(temp_cp), "%d %d %s", node->num, sa_val, "atpg_tp.txt");
//     cp = temp_cp;
//     pattern.clear();
//     if (use_podem) {
//         podem();
//     } else {
//         dalg();
//     }
//     // if (!fault_reached_output)
//     //     return false;
//     FILE* f = fopen("atpg_tp.txt", "r");
//     if (!f) return false;
//     char line[1024];
//     fgets(line, sizeof(line), f);

//     while (fgets(line, sizeof(line), f)) {
//         if (strlen(line) > 2) {
//             char* token = strtok(line, ",\n");
//             while (token && pattern.size() < Npi) {
//                 if (strcmp(token, "0") == 0) pattern.push_back(0);
//                 else if (strcmp(token, "1") == 0) pattern.push_back(1);
//                 else pattern.push_back(2);
//                 token = strtok(NULL, ",\n");
//             }
//             break;
//         }
        
//     }
//     fclose(f);
//     // printf("PATTERN: ");
//     if (!pattern.empty()) {
//         // for (int b : pattern) {
//         //     printf("%c", b == 0 ? '0' : b == 1 ? '1' : 'x');
//         // }
//         // printf("\n");
//     } else {
//         // printf("No pattern generated\n");
//         return false;
//     }
//     return true;
// }


// extern char* cp;
// extern NSTRUC** Pinput;
// extern int Npi;

// void run_fs(const std::vector<int>& pattern, bool use_dfs, std::vector<std::pair<NSTRUC*, int>>& detected) {
//     const char* in_file = "fs_input.txt";
//     const char* out_file = "fs_output.txt";

//     // Write input pattern to file
//     std::ofstream f(in_file);
//     for (int i = 0; i < Npi; ++i) {
//         f << Pinput[i]->num;
//         if (i < Npi - 1) f << ",";
//     }
//     f << "\n";
//     for (int i = 0; i < Npi; ++i) {
//         if (pattern[i] == 0) f << "0";
//         else if (pattern[i] == 1) f << "1";
//         else f << "2";  // for 'x'
//         if (i < Npi - 1) f << ",";
//     }
//     f << "\n";
//     f.close();

//     // Set cp and call DFS or PFS
//     static char fs_cp[256];

//     if (use_dfs) {
//         snprintf(fs_cp, sizeof(fs_cp), "%s %s", in_file, out_file);
//         cp = fs_cp;
//         printf("      Running DFS...\n");
//         dfs();
//     } else {
//         std::ofstream pfl("pfs_fl.txt");
//         for (int i = 0; i < Nnodes; ++i) {
//             pfl << Node[i].num << "@0\n";
//             pfl << Node[i].num << "@1\n";
//         }
//         pfl.close();
//         snprintf(fs_cp, sizeof(fs_cp), "%s %s %s", in_file, "pfs_fl.txt", out_file);
//         cp = fs_cp;
//         printf("      Running PFS...\n");
//         pfs();
//     }

//     // Read back detected faults
//     detected.clear();
//     std::ifstream fin(out_file);
//     std::string line;
//     while (std::getline(fin, line)) {
//         size_t at = line.find('@');
//         if (at != std::string::npos) {
//             int node_num = std::stoi(line.substr(0, at));
//             int sa_val = std::stoi(line.substr(at + 1));
//             for (int i = 0; i < Nnodes; ++i) {
//                 if (Node[i].num == node_num) {
//                     detected.emplace_back(&Node[i], sa_val);
//                     break;
//                 }
//             }
//         }
//     }
//     fin.close();
// }

// void load_pattern_from_file(const char* filename, std::vector<int>& pattern) {
//     FILE* f = fopen(filename, "r");
//     if (!f) return;
//     char line[MAXLINE];
//     while (fgets(line, sizeof(line), f)) {
//         if (strlen(line) > 2) {
//             pattern.clear();
//             for (int i = 0; i < Npi; i++) {
//                 if (line[i] == '0') pattern.push_back(0);
//                 else if (line[i] == '1') pattern.push_back(1);
//                 else pattern.push_back(2);  // 'x'
//             }
//             break;
//         }
//     }
//     fclose(f);
// }

// std::vector<std::pair<NSTRUC*, int>> load_rfl_faults(const char* filename) {
//     std::vector<std::pair<NSTRUC*, int>> rfl;
//     FILE* f = fopen(filename, "r");
//     if (!f) {
//         printf("Warning: Could not open RFL file: %s\n", filename);
//         return rfl;
//     }

//     char line[256];
//     while (fgets(line, sizeof(line), f)) {
//         int node_num, sa_val;
//         if (sscanf(line, "%d@%d", &node_num, &sa_val) == 2) {
//             for (int i = 0; i < Nnodes; ++i) {
//                 if (Node[i].num == node_num) {
//                     rfl.emplace_back(&Node[i], sa_val);
//                     break;
//                 }
//             }
//         }
//     }

//     fclose(f);
//     return rfl;
// }

// std::string dfrontier_mode = "";
// std::string jfrontier_mode = "";

// // TPG command
// void tpg() {
//     std::unordered_set<std::string> tested_patterns;
//     auto tpg_start = std::chrono::high_resolution_clock::now();
//     char atpg_type[MAXLINE], fs_type[MAXLINE], output_file[MAXLINE];
//     static std::vector<std::pair<NSTRUC*, int>> undetectable_faults;
//     undetectable_faults.clear();
//     if (sscanf(cp, "%s %s %s", atpg_type, fs_type, output_file) != 3) {
//         printf("Usage: TPG <PODEM|DALG> <DFS|PFS> <output-file>\n");
//         return;
//     }
//     reset_d_values();
//     fault_reached_output = false;
//     bool use_podem = strcmp(atpg_type, "PODEM") == 0;
//     bool use_dfs   = strcmp(fs_type,   "DFS")   == 0;
//     bool use_rfl   = strstr(cp, "-fl rfl") != NULL;

//     bool use_rtpg_v1 = false;
//     bool use_rtpg_v2 = false;
//     bool use_rtpg_v3 = false;
//     bool use_rtpg_v4 = false;
//     char* use_tvc = strstr(cp, "-tvc");
//     float rtpg_target_fc = 0.0;
//     float delta_fc_threshold = 0.0f;
//     char* rtp_flag = strstr(cp, "-rtp");
//     if (rtp_flag && strstr(cp, "v1")) {
//         use_rtpg_v1 = true;
//         sscanf(rtp_flag, "-rtp v1 %f", &rtpg_target_fc);
//     }
//     if (rtp_flag && strstr(cp, "v2")) {
//         use_rtpg_v2 = true;
//         sscanf(rtp_flag, "-rtp v2 %f", &delta_fc_threshold);
//     }
//     if (rtp_flag && strstr(cp, "v3")) {
//         use_rtpg_v3 = true;
//         sscanf(rtp_flag, "-rtp v3 %f", &delta_fc_threshold);
//     }
//     if (rtp_flag && strstr(cp, "v4")) {
//         use_rtpg_v4 = true;
//         sscanf(rtp_flag, "-rtp v4 %f", &delta_fc_threshold);
//     }
//     char* use_df = strstr(cp, "-df");
//     if (use_df) {
//         char temp[10];
//         sscanf(use_df, "-df %s", temp);
//         dfrontier_mode = temp;
//     }
//     char* use_jf = strstr(cp, "-jf");
//     if (use_jf) {
//         char temp[10];
//         sscanf(use_jf, "-jf %s", temp);
//         jfrontier_mode = temp;
//         // printf("Jfrontier mode: %s\n", jfrontier_mode.c_str());
//     }
//     std::vector<std::pair<NSTRUC*, int>> fault_list = generate_fault_list();
//     std::vector<std::vector<int>> pattern_list;
    
//     std::unordered_set<int> rfl_keys;

//     if (use_rfl) {
//         char original_cp[MAXLINE];
//         strncpy(original_cp, cp, MAXLINE);
//         cp = (char*)"rfl.txt";
//         rfl();
//         cp = original_cp;
//         std::vector<std::pair<NSTRUC*, int>> rfl_faults = load_rfl_faults("rfl.txt");
//         for (auto& f : rfl_faults) {
//             int key = f.first->num * 2 + f.second;
//             rfl_keys.insert(key);
//         }
//     }
//     printf("Total faults in FL: %zu\n", fault_list.size());
//     if (use_rfl) {
//         printf("Total faults in RFL: %zu\n", rfl_keys.size());
//     }

//     float prev_fc = 0.0f;
//     std::vector<std::pair<NSTRUC*, int>> initial_faults = fault_list;
    
//     float total_faults = 0;
//     float current_fc = 0;
//     float delta_fc = 0;
//     // std::vector<std::pair<NSTRUC*, int>> undetectable_faults;
//     if ((use_rtpg_v1 && rtpg_target_fc > 0.0f)||use_rtpg_v3||use_rtpg_v4) {
//         if (use_rtpg_v1) printf("Running RTPG to reach %.2f%% fault coverage...\n", rtpg_target_fc);
//         if (use_rtpg_v3) printf("Running RTPG to reach ΔFC < %.2f%%...\n", delta_fc_threshold);
//         if (use_rtpg_v4) printf("Running RTPG v4...\n");
//         // Generate test patterns to file
//         cp = (char*)"500 t rtpg_generated.tp"; 
//         printf("Calling rtpg()...\n");
//         rtpg();
//         std::ifstream infile("rtpg_generated.tp");
//         std::string header;
//         std::getline(infile, header); 
//         std::string line;
//         std::vector<std::pair<NSTRUC*, int>> remaining_faults = fault_list;
//         int M = 10;
//         int pattern_count = 0;
//         while (std::getline(infile, line)) {
//             if (use_rtpg_v4) {
//                 printf("   Running RTPG v4 (best of %d candidates)...\n", M);
//                 int best_detected = -1;
//                 std::vector<int> best_pattern;
//                 std::vector<std::pair<NSTRUC*, int>> best_faults;

//                 for (int trial = 0; trial < M; ++trial) {
//                     std::vector<int> pattern;
//                     std::string trial_line;
//                     if (!std::getline(infile, trial_line)) break;
//                     std::istringstream iss(trial_line);
//                     std::string tok;
//                     while (std::getline(iss, tok, ',')) {
//                         if (tok == "0") pattern.push_back(0);
//                         else if (tok == "1") pattern.push_back(1);
//                         else pattern.push_back(2);
//                     }

//                     std::string pattern_str;
//                     for (int bit : pattern) {
//                         if (bit == 0) pattern_str += '0';
//                         else if (bit == 1) pattern_str += '1';
//                         else pattern_str += 'x';
//                     }
//                     if (tested_patterns.count(pattern_str)) {
//                         continue; // Skip if already tested
//                     }

//                     std::vector<std::pair<NSTRUC*, int>> detected;
//                     run_fs(pattern, use_dfs, detected);
//                     if ((int)detected.size() > best_detected) {
//                         best_detected = detected.size();
//                         best_pattern = pattern;
//                         best_faults = detected;
//                     }
//                 }

//                 if (!best_pattern.empty()) {
//                     std::string best_pattern_str;
//                     for (int bit : best_pattern) {
//                         if (bit == 0) best_pattern_str += '0';
//                         else if (bit == 1) best_pattern_str += '1';
//                         else best_pattern_str += 'x';
//                     }
//                     tested_patterns.insert(best_pattern_str);

//                     pattern_list.push_back(best_pattern);
//                     for (auto& f : best_faults) {
//                         remaining_faults.erase(
//                             std::remove(remaining_faults.begin(), remaining_faults.end(), f),
//                             remaining_faults.end()
//                         );
//                     }

//                     total_faults = initial_faults.size();
//                     current_fc = 100.0f * (total_faults - remaining_faults.size() - undetectable_faults.size()) / total_faults;
//                     delta_fc = current_fc - prev_fc;
//                     prev_fc = current_fc;
//                     float fc = 100.0f * (fault_list.size() - remaining_faults.size()) / fault_list.size();
//                     printf("RTPG v4 #%d → FC = %.2f%%, ΔFC: %.2f%% (%zu faults remaining)\n\n", ++pattern_count, fc, delta_fc, remaining_faults.size());
//                     if (delta_fc <= delta_fc_threshold) {
//                         printf("ΔFC = %.2f%% is below threshold %.2f%%.  Exiting...\n", delta_fc, delta_fc_threshold);
//                         break;
//                     }
//                 }

//             } else { 


            
//                 printf("   Running RTPG...\n");
//                 std::vector<int> pattern;
//                 std::istringstream iss(line);
//                 std::string tok;
//                 while (std::getline(iss, tok, ',')) {
//                     if (tok == "0") pattern.push_back(0);
//                     else if (tok == "1") pattern.push_back(1);
//                     else pattern.push_back(2);
//                 }


//                 std::string pattern_str;
//                 for (int bit : pattern) {
//                     if (bit == 0) pattern_str += '0';
//                     else if (bit == 1) pattern_str += '1';
//                     else pattern_str += 'x';
//                 }
//                 if (tested_patterns.count(pattern_str)) {
//                     printf("      Pattern already tested: %s, Regenerating...\n", pattern_str.c_str());
//                     continue;
//                 }
//                 tested_patterns.insert(pattern_str);


//                 std::vector<std::pair<NSTRUC*, int>> detected;
//                 run_fs(pattern, use_dfs, detected);
//                 if (!detected.empty()) {
//                     pattern_list.push_back(pattern);
//                     for (auto& f : detected) {
//                         remaining_faults.erase(
//                             std::remove(remaining_faults.begin(), remaining_faults.end(), f),
//                             remaining_faults.end()
//                         );
//                     }
//                 }
//                 total_faults = initial_faults.size();
//                 current_fc = 100.0f * (total_faults - remaining_faults.size() - undetectable_faults.size()) / total_faults;
//                 delta_fc = current_fc - prev_fc;
//                 prev_fc = current_fc;
//                 float fc = 100.0f * (fault_list.size() - remaining_faults.size()) / fault_list.size();
//                 printf("RTPG #%d → FC = %.2f%%, ΔFC: %.2f%% (%zu faults remaining)\n\n", ++pattern_count, fc, delta_fc, remaining_faults.size());
//                 if ((fc >= rtpg_target_fc)&&use_rtpg_v1) {
//                     printf("Reached target FC %.2f%% — switching to ATPG\n\n", fc);
//                     break;
//                 }
//                 if ((delta_fc <= delta_fc_threshold)&&use_rtpg_v3) {
//                     printf("ΔFC = %.2f%% is below threshold %.2f%%.  Exiting...\n", delta_fc, delta_fc_threshold);
//                     break;
//                 }
//             }
//         }
//         infile.close();
//         fault_list = remaining_faults;
//         // initial_faults = fault_list;
//         prev_fc = 100.0f * (initial_faults.size() - fault_list.size() - undetectable_faults.size()) / initial_faults.size();

//     }
//     int pattern_count = 0;
//     while ((!fault_list.empty())&&(!use_rtpg_v3)&&(!use_rtpg_v4)) {
//         auto fault = fault_list.back();
//         fault_list.pop_back();
//         // printf("DEBUG: Next fault = node %d stuck-at-%d\n", fault.first->num, fault.second);
//         int key = fault.first->num * 2 + fault.second;
//         if (use_rfl && rfl_keys.count(key) == 0 && !rfl_keys.empty()) {
//             printf("   Fault %d@%d not in RFL, skipping...\n", fault.first->num, fault.second);
//             continue; // skip non-RFL faults while RFL is unfinished
//         }
//         // printf("Generating test pattern for fault at node %d with value %d\n", fault.first->num, fault.second);

//         std::vector<int> pattern;
//         bool success = false;

//         // Call ATPG
//         // printf("DEBUG: Running ATPG for node %d stuck-at-%d\n", fault.first ? fault.first->num : -1, fault.second);
//         printf("   Running ATPG... (%d@%d)\n", fault.first->num, fault.second);
//         success = run_atpg(fault.first, fault.second, pattern, use_podem);
//         if (!success) {
//             // printf("      No Test Pattern Generated for Fault %d@%d\n\n", fault.first->num, fault.second);
//             delta_fc = 0;
//             printf("      No Test Pattern Generated \n");
//             printf("ATPG #%d → FC = %.2f%%, ΔFC: %.2f%% (%zu faults remaining)\n\n", ++pattern_count, current_fc, delta_fc, fault_list.size());
//             undetectable_faults.push_back(fault);
//             continue;
//         }

// // READ ../ckts_all/c499.ckt
// // TPG PODEM DFS my_patterns.tp -rtp v1 30

//         if (use_tvc) {
//             printf("   Optimizing pattern with TVC...\n");
//             for (int i = 0; i < pattern.size(); ++i) {
//                 if (pattern[i] == 2) { // Found an X
//                     // Try setting to 0
//                     pattern[i] = 0;
//                     std::vector<std::pair<NSTRUC*, int>> detected0;
//                     run_fs(pattern, use_dfs, detected0);

//                     // Try setting to 1
//                     pattern[i] = 1;
//                     std::vector<std::pair<NSTRUC*, int>> detected1;
//                     run_fs(pattern, use_dfs, detected1);

//                     // Choose better
//                     if (detected1.size() > detected0.size()) {
//                         pattern[i] = 1; // keep as 1
//                     } else {
//                         pattern[i] = 0; // keep as 0
//                     }
//                     printf("      Optimized pattern: ");
//                     for (int b : pattern) {
//                         printf("%c", b == 0 ? '0' : b == 1 ? '1' : 'x');
//                     }
//                     printf("\n");
//                 }
//             }
//         }


//         pattern_list.push_back(pattern);
//         // Fault simulation
//         std::vector<std::pair<NSTRUC*, int>> detected;
//         run_fs(pattern, use_dfs, detected);

//         // Remove detected faults from the list
//         for (auto& f : detected) {
//             fault_list.erase(
//                 std::remove(fault_list.begin(), fault_list.end(), f),
//                 fault_list.end()
//             );
//         }
//         // Calculate fault coverage delta
//         total_faults = initial_faults.size();
//         current_fc = 100.0f * (total_faults - fault_list.size() - undetectable_faults.size()) / total_faults;
//         delta_fc = current_fc - prev_fc;
//         prev_fc = current_fc;
//         printf("ATPG #%d → FC = %.2f%%, ΔFC: %.2f%% (%zu faults remaining)\n\n", ++pattern_count, current_fc, delta_fc, fault_list.size());

//         // printf("DEBUG: Checking if ΔFC %.2f < threshold %.2f\n", delta_fc, delta_fc_threshold);

//         // Exit ATPG if delta FC too small
//         if (use_rtpg_v2 && delta_fc < delta_fc_threshold) {
//             printf("ΔFC = %.2f%% is below threshold %.2f%%.  Exiting...\n", delta_fc, delta_fc_threshold);
//             break;
//         }
//     }
//     int detected_faults = initial_faults.size() - fault_list.size() - undetectable_faults.size();
//     float final_fc = 100.0f * detected_faults / initial_faults.size();
//     printf("Final Fault Coverage = %.2f%%\n", final_fc);
//     printf("Detected: %d, Undetectable: %zu, Remaining: %zu\n",
//        detected_faults, undetectable_faults.size(), fault_list.size());


//     // Write the generated patterns to a .tp file
//     // printf("DEBUG: Writing %zu patterns to output file\n", pattern_list.size());
//     std::ofstream fout(output_file);
//     for (const auto& p : pattern_list) {
//         for (int i = 0; i < p.size(); ++i) {
//             if (p[i] == 0) fout << "0";
//             else if (p[i] == 1) fout << "1";
//             else fout << "x";
//             if (i < p.size() - 1) fout << ",";
//         }
//         fout << '\n';
//     }
//     fout.close();       
//     // printf("TPG complete. %zu test patterns written to tpg_output.tp\n", pattern_list.size());
//     printf("TPG complete. %zu test patterns written to %s\n", pattern_list.size(), output_file);
//     auto tpg_end = std::chrono::high_resolution_clock::now();
//     double elapsed_sec = std::chrono::duration<double>(tpg_end - tpg_start).count();
//     printf("TPG time elapsed: %.3f seconds\n", elapsed_sec); 

// }

// // READ ../auto-tests-phase3/ckts/c1.ckt
// // TPG PODEM DFS my_patterns.tp
// // TPG DALG DFS my_patterns.tp

// // READ ../auto-tests-phase3/ckts/c1.ckt
// // TPG PODEM DFS my_patterns.tp -fl rfl

// // READ ../ckts_all/c6288.ckt
// // TPG DALG PFS my_patterns.tp
// // TPG PODEM PFS my_patterns.tp -rtp v1 80
// // TPG PODEM PFS my_patterns.tp -rtp v2 1
// // TPG PODEM PFS my_patterns.tp -rtp v3 0.1
// // TPG PODEM PFS my_patterns.tp -rtp v4 20

// // READ ../ckts_all/c432.ckt
// // TPG PODEM PFS my_patterns.tp -rtp v2 0.00001
// // TPG PODEM PFS my_patterns.tp -rtp v2 0.00001 -tvc

// // READ ../ckts_all/c432.ckt
// // TPG PODEM PFS my_patterns.tp -rtp v1 30

// // READ ../ckts_all/c6288.ckt
// // TPG PODEM PFS my_patterns.tp -df nl
// // TPG PODEM PFS my_patterns.tp -df nh
// // TPG PODEM PFS my_patterns.tp -df lh
// // TPG PODEM PFS my_patterns.tp -df cc

// // READ ../ckts_all/c6288.ckt
// // TPG PODEM PFS my_patterns.tp -jf v0
// // TPG PODEM PFS my_patterns.tp

// // READ ../ckts_all/c6288.ckt
// // TPG PODEM PFS my_patterns.tp -fl rfl










// // PFS:
// // Final Fault Coverage = 99.27%
// // Detected: 12484, Undetectable: 92, Remaining: 0
// // TPG complete. 61 test patterns written to tpg_output.tp
// // TPG time elapsed: 128.264 seconds

// // DFS:
// // Final Fault Coverage = 99.31%
// // Detected: 12489, Undetectable: 87, Remaining: 0
// // TPG complete. 68 test patterns written to tpg_output.tp
// // TPG time elapsed: 1673.199 seconds





// // NO TVC: (c432, rtp v2 0.00001)
// // Final Fault Coverage = 99.42%
// // Detected: 859, Undetectable: 5, Remaining: 0
// // TPG complete. 78 test patterns written to my_patterns.tp
// // TPG time elapsed: 0.683 seconds

// // TVC: (c432, rtp v2 0.00001)
// // Final Fault Coverage = 99.42%
// // Detected: 859, Undetectable: 5, Remaining: 0
// // TPG complete. 69 test patterns written to my_patterns.tp
// // TPG time elapsed: 14.422 seconds



// // DALG, c6288, Baseline:
// // Final Fault Coverage = 98.23%
// // Detected: 12354, Undetectable: 222, Remaining: 0
// // TPG complete. 169 test patterns written to my_patterns.tp
// // TPG time elapsed: 640.244 seconds








// // HEURISTICS (NL): Lowest Node Number
// // Final Fault Coverage = 99.46%
// // Detected: 12508, Undetectable: 68, Remaining: 0
// // TPG complete. 101 test patterns written to my_patterns.tp
// // TPG time elapsed: 176.997 seconds

// // HEURISTICS (NH):  Highest Node Number
// // Final Fault Coverage = 99.46%
// // Detected: 12508, Undetectable: 68, Remaining: 0
// // TPG complete. 75 test patterns written to my_patterns.tp
// // TPG time elapsed: 135.388 seconds

// // HEURISTICS (LH):  Highest Level
// // Final Fault Coverage = 99.46%
// // Detected: 12508, Undetectable: 68, Remaining: 0
// // TPG complete. 75 test patterns written to my_patterns.tp
// // TPG time elapsed: 158.254 seconds

// // HEURISTICS (CC):  Lowest Scoap Observability
// // Final Fault Coverage = 99.46%
// // Detected: 12508, Undetectable: 68, Remaining: 0
// // TPG complete. 78 test patterns written to my_patterns.tp
// // TPG time elapsed: 144.579 seconds













#include "circuit.h"
#include "dalg.h"
#include "podem.h"
#include "dfs.h"
#include "pfs.h"
#include "tpg.h"
#include "rfl.h"
#include "rtpg.h"
#include "scoap.h"
#include <vector>
#include <fstream>
#include <algorithm>
#include <chrono>

static bool use_podem = true;  // false = use DALG
static bool use_dfs = true;    // false = use PFS

// Generates the full fault list
std::vector<std::pair<NSTRUC*, int>> generate_fault_list() {
    std::vector<std::pair<NSTRUC*, int>> flist;
    std::ofstream fout("fault_list.txt");
    for (int i = 0; i < Nnodes; ++i) {
        NSTRUC* np = &Node[i];
        flist.emplace_back(np, 0);
        flist.emplace_back(np, 1);
        fout << np->num << "@0\n";
        fout << np->num << "@1\n";
    }
    fout.close();
    return flist;
}

extern char* cp;
extern bool fault_reached_output;
extern int Npi;
extern NSTRUC** Pinput;


bool run_atpg(NSTRUC* node, int sa_val, std::vector<int>& pattern, bool use_podem) {
    static char temp_cp[128];
    snprintf(temp_cp, sizeof(temp_cp), "%d %d %s", node->num, sa_val, "atpg_tp.txt");
    cp = temp_cp;
    pattern.clear();
    if (use_podem) {
        podem();
    } else {
        dalg();
    }
    // if (!fault_reached_output)
    //     return false;
    FILE* f = fopen("atpg_tp.txt", "r");
    if (!f) return false;
    char line[1024];
    fgets(line, sizeof(line), f);

    while (fgets(line, sizeof(line), f)) {
        if (strlen(line) > 2) {
            char* token = strtok(line, ",\n");
            while (token && pattern.size() < Npi) {
                if (strcmp(token, "0") == 0) pattern.push_back(0);
                else if (strcmp(token, "1") == 0) pattern.push_back(1);
                else pattern.push_back(2);
                token = strtok(NULL, ",\n");
            }
            break;
        }
        
    }
    fclose(f);
    // printf("PATTERN: ");
    if (!pattern.empty()) {
        // for (int b : pattern) {
        //     printf("%c", b == 0 ? '0' : b == 1 ? '1' : 'x');
        // }
        // printf("\n");
    } else {
        // printf("No pattern generated\n");
        return false;
    }
    return true;
}


extern char* cp;
extern NSTRUC** Pinput;
extern int Npi;

void run_fs(const std::vector<int>& pattern, bool use_dfs, std::vector<std::pair<NSTRUC*, int>>& detected) {
    const char* in_file = "fs_input.txt";
    const char* out_file = "fs_output.txt";

    // Write input pattern to file
    std::ofstream f(in_file);
    for (int i = 0; i < Npi; ++i) {
        f << Pinput[i]->num;
        if (i < Npi - 1) f << ",";
    }
    f << "\n";
    for (int i = 0; i < Npi; ++i) {
        if (pattern[i] == 0) f << "0";
        else if (pattern[i] == 1) f << "1";
        else f << "2";  // for 'x'
        if (i < Npi - 1) f << ",";
    }
    f << "\n";
    f.close();

    // Set cp and call DFS or PFS
    static char fs_cp[256];

    if (use_dfs) {
        snprintf(fs_cp, sizeof(fs_cp), "%s %s", in_file, out_file);
        cp = fs_cp;
        printf("      Running DFS...\n");
        dfs();
    } else {
        std::ofstream pfl("pfs_fl.txt");
        for (int i = 0; i < Nnodes; ++i) {
            pfl << Node[i].num << "@0\n";
            pfl << Node[i].num << "@1\n";
        }
        pfl.close();
        snprintf(fs_cp, sizeof(fs_cp), "%s %s %s", in_file, "pfs_fl.txt", out_file);
        cp = fs_cp;
        printf("      Running PFS...\n");
        pfs();
    }

    // Read back detected faults
    detected.clear();
    std::ifstream fin(out_file);
    std::string line;
    while (std::getline(fin, line)) {
        size_t at = line.find('@');
        if (at != std::string::npos) {
            int node_num = std::stoi(line.substr(0, at));
            int sa_val = std::stoi(line.substr(at + 1));
            for (int i = 0; i < Nnodes; ++i) {
                if (Node[i].num == node_num) {
                    detected.emplace_back(&Node[i], sa_val);
                    break;
                }
            }
        }
    }
    fin.close();
}

void load_pattern_from_file(const char* filename, std::vector<int>& pattern) {
    FILE* f = fopen(filename, "r");
    if (!f) return;
    char line[MAXLINE];
    while (fgets(line, sizeof(line), f)) {
        if (strlen(line) > 2) {
            pattern.clear();
            for (int i = 0; i < Npi; i++) {
                if (line[i] == '0') pattern.push_back(0);
                else if (line[i] == '1') pattern.push_back(1);
                else pattern.push_back(2);  // 'x'
            }
            break;
        }
    }
    fclose(f);
}

std::vector<std::pair<NSTRUC*, int>> load_rfl_faults(const char* filename) {
    std::vector<std::pair<NSTRUC*, int>> rfl;
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Warning: Could not open RFL file: %s\n", filename);
        return rfl;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        int node_num, sa_val;
        if (sscanf(line, "%d@%d", &node_num, &sa_val) == 2) {
            for (int i = 0; i < Nnodes; ++i) {
                if (Node[i].num == node_num) {
                    rfl.emplace_back(&Node[i], sa_val);
                    break;
                }
            }
        }
    }

    fclose(f);
    return rfl;
}

int get_fault_difficulty(NSTRUC* node, int sa_val) {
    int cc = (sa_val == 0) ? node->cc1 : node->cc0;
    return cc + node->co;
}


std::string dfrontier_mode = "";
std::string jfrontier_mode = "";

// TPG command
void tpg() {
    std::unordered_set<std::string> tested_patterns;
    auto tpg_start = std::chrono::high_resolution_clock::now();
    char atpg_type[MAXLINE], fs_type[MAXLINE], output_file[MAXLINE];
    static std::vector<std::pair<NSTRUC*, int>> undetectable_faults;
    undetectable_faults.clear();
    if (sscanf(cp, "%s %s %s", atpg_type, fs_type, output_file) != 3) {
        printf("Usage: TPG <PODEM|DALG> <DFS|PFS> <output-file>\n");
        return;
    }
    reset_d_values();
    fault_reached_output = false;
    bool use_podem = strcmp(atpg_type, "PODEM") == 0;
    bool use_dfs   = strcmp(fs_type,   "DFS")   == 0;
    bool use_rfl   = strstr(cp, "-fl rfl") != NULL;
    bool use_scoap = strstr(cp, "-fl scoap") != NULL;
    bool scoap_hard_mode = strstr(cp, "-fl scoap HD") != NULL;
    if (use_scoap) {
        char original_cp[MAXLINE];
        strncpy(original_cp, cp, MAXLINE);
        cp = (char*)"scoap.out";
        scoap();
        cp = original_cp;
    }


    bool use_rtpg_v1 = false;
    bool use_rtpg_v2 = false;
    bool use_rtpg_v3 = false;
    bool use_rtpg_v4 = false;
    char* use_tvc = strstr(cp, "-tvc");
    float rtpg_target_fc = 0.0;
    float delta_fc_threshold = 0.0f;
    char* rtp_flag = strstr(cp, "-rtp");
    if (rtp_flag && strstr(cp, "v1")) {
        use_rtpg_v1 = true;
        sscanf(rtp_flag, "-rtp v1 %f", &rtpg_target_fc);
    }
    if (rtp_flag && strstr(cp, "v2")) {
        use_rtpg_v2 = true;
        sscanf(rtp_flag, "-rtp v2 %f", &delta_fc_threshold);
    }
    if (rtp_flag && strstr(cp, "v3")) {
        use_rtpg_v3 = true;
        sscanf(rtp_flag, "-rtp v3 %f", &delta_fc_threshold);
    }
    if (rtp_flag && strstr(cp, "v4")) {
        use_rtpg_v4 = true;
        sscanf(rtp_flag, "-rtp v4 %f", &delta_fc_threshold);
    }
    char* use_df = strstr(cp, "-df");
    if (use_df) {
        char temp[10];
        sscanf(use_df, "-df %s", temp);
        dfrontier_mode = temp;
    }
    char* use_jf = strstr(cp, "-jf");
    if (use_jf) {
        char temp[10];
        sscanf(use_jf, "-jf %s", temp);
        jfrontier_mode = temp;
        // printf("Jfrontier mode: %s\n", jfrontier_mode.c_str());
    }
    std::vector<std::pair<NSTRUC*, int>> fault_list = generate_fault_list();

    if (use_scoap) {
        std::sort(fault_list.begin(), fault_list.end(), [scoap_hard_mode](const std::pair<NSTRUC*, int>& a, const std::pair<NSTRUC*, int>& b) {
            int scoap_a = (a.second == 0) ? a.first->cc1 + a.first->co : a.first->cc0 + a.first->co;
            int scoap_b = (b.second == 0) ? b.first->cc1 + b.first->co : b.first->cc0 + b.first->co;
            return scoap_hard_mode ? (scoap_a < scoap_b) : (scoap_a > scoap_b);
        });
        printf("\nSorted fault list using SCOAP (%s first).\n\n", scoap_hard_mode ? "Hardest" : "Easiest");
        // for (const auto& fault : fault_list) {
        //     int scoap_val = (fault.second == 0) ? fault.first->cc1 + fault.first->co : fault.first->cc0 + fault.first->co;
        //     printf("   Fault %d@%d → CC+CO = %d\n", fault.first->num, fault.second, scoap_val);
        // }
    }
// READ ../ckts_all/c6288.ckt
// TPG PODEM PFS my_patterns.tp -fl scoap
    std::vector<std::vector<int>> pattern_list;
    
    std::unordered_set<int> rfl_keys;

    if (use_rfl) {
        char original_cp[MAXLINE];
        strncpy(original_cp, cp, MAXLINE);
        cp = (char*)"rfl.txt";
        rfl();
        cp = original_cp;
        system("cp rfl.txt debug_rfl.txt");
        std::vector<std::pair<NSTRUC*, int>> rfl_faults = load_rfl_faults("rfl.txt");
        for (auto& f : rfl_faults) {
            int key = f.first->num * 2 + f.second;
            rfl_keys.insert(key);
        }
    }
    printf("Total faults in FL: %zu\n", fault_list.size());
    if (use_rfl) {
        printf("Total faults in RFL: %zu\n", rfl_keys.size());
    }

    float prev_fc = 0.0f;
    std::vector<std::pair<NSTRUC*, int>> initial_faults = fault_list;
    
    float total_faults = 0;
    float current_fc = 0;
    float delta_fc = 0;
    // std::vector<std::pair<NSTRUC*, int>> undetectable_faults;
    if ((use_rtpg_v1 && rtpg_target_fc > 0.0f)||use_rtpg_v3||use_rtpg_v4) {
        if (use_rtpg_v1) printf("Running RTPG to reach %.2f%% fault coverage...\n", rtpg_target_fc);
        if (use_rtpg_v3) printf("Running RTPG to reach ΔFC < %.2f%%...\n", delta_fc_threshold);
        if (use_rtpg_v4) printf("Running RTPG v4...\n");
        // Generate test patterns to file
        cp = (char*)"500 t rtpg_generated.tp"; 
        printf("Calling rtpg()...\n");
        rtpg();
        std::ifstream infile("rtpg_generated.tp");
        std::string header;
        std::getline(infile, header); 
        std::string line;
        std::vector<std::pair<NSTRUC*, int>> remaining_faults = fault_list;
        int M = 10;
        int pattern_count = 0;
        while (std::getline(infile, line)) {
            if (use_rtpg_v4) {
                printf("   Running RTPG v4 (best of %d candidates)...\n", M);
                int best_detected = -1;
                std::vector<int> best_pattern;
                std::vector<std::pair<NSTRUC*, int>> best_faults;

                for (int trial = 0; trial < M; ++trial) {
                    std::vector<int> pattern;
                    std::string trial_line;
                    if (!std::getline(infile, trial_line)) break;
                    std::istringstream iss(trial_line);
                    std::string tok;
                    while (std::getline(iss, tok, ',')) {
                        if (tok == "0") pattern.push_back(0);
                        else if (tok == "1") pattern.push_back(1);
                        else pattern.push_back(2);
                    }

                    std::string pattern_str;
                    for (int bit : pattern) {
                        if (bit == 0) pattern_str += '0';
                        else if (bit == 1) pattern_str += '1';
                        else pattern_str += 'x';
                    }
                    if (tested_patterns.count(pattern_str)) {
                        continue; // Skip if already tested
                    }

                    std::vector<std::pair<NSTRUC*, int>> detected;
                    run_fs(pattern, use_dfs, detected);
                    if ((int)detected.size() > best_detected) {
                        best_detected = detected.size();
                        best_pattern = pattern;
                        best_faults = detected;
                    }
                }

                if (!best_pattern.empty()) {
                    std::string best_pattern_str;
                    for (int bit : best_pattern) {
                        if (bit == 0) best_pattern_str += '0';
                        else if (bit == 1) best_pattern_str += '1';
                        else best_pattern_str += 'x';
                    }
                    tested_patterns.insert(best_pattern_str);

                    pattern_list.push_back(best_pattern);
                    for (auto& f : best_faults) {
                        remaining_faults.erase(
                            std::remove(remaining_faults.begin(), remaining_faults.end(), f),
                            remaining_faults.end()
                        );
                    }

                    total_faults = initial_faults.size();
                    current_fc = 100.0f * (total_faults - remaining_faults.size() - undetectable_faults.size()) / total_faults;
                    delta_fc = current_fc - prev_fc;
                    prev_fc = current_fc;
                    float fc = 100.0f * (fault_list.size() - remaining_faults.size()) / fault_list.size();
                    printf("RTPG v4 #%d → FC = %.2f%%, ΔFC: %.2f%% (%zu faults remaining)\n\n", ++pattern_count, fc, delta_fc, remaining_faults.size());
                    if (delta_fc <= delta_fc_threshold) {
                        printf("ΔFC = %.2f%% is below threshold %.2f%%.  Exiting...\n", delta_fc, delta_fc_threshold);
                        break;
                    }
                }

            } else { 


            
                printf("   Running RTPG...\n");
                std::vector<int> pattern;
                std::istringstream iss(line);
                std::string tok;
                while (std::getline(iss, tok, ',')) {
                    if (tok == "0") pattern.push_back(0);
                    else if (tok == "1") pattern.push_back(1);
                    else pattern.push_back(2);
                }


                std::string pattern_str;
                for (int bit : pattern) {
                    if (bit == 0) pattern_str += '0';
                    else if (bit == 1) pattern_str += '1';
                    else pattern_str += 'x';
                }
                if (tested_patterns.count(pattern_str)) {
                    printf("      Pattern already tested: %s, Regenerating...\n", pattern_str.c_str());
                    continue;
                }
                tested_patterns.insert(pattern_str);


                std::vector<std::pair<NSTRUC*, int>> detected;
                run_fs(pattern, use_dfs, detected);
                if (!detected.empty()) {
                    pattern_list.push_back(pattern);
                    for (auto& f : detected) {
                        remaining_faults.erase(
                            std::remove(remaining_faults.begin(), remaining_faults.end(), f),
                            remaining_faults.end()
                        );
                    }
                }
                total_faults = initial_faults.size();
                current_fc = 100.0f * (total_faults - remaining_faults.size() - undetectable_faults.size()) / total_faults;
                delta_fc = current_fc - prev_fc;
                prev_fc = current_fc;
                float fc = 100.0f * (fault_list.size() - remaining_faults.size()) / fault_list.size();
                printf("RTPG #%d → FC = %.2f%%, ΔFC: %.2f%% (%zu faults remaining)\n\n", ++pattern_count, fc, delta_fc, remaining_faults.size());
                if ((fc >= rtpg_target_fc)&&use_rtpg_v1) {
                    printf("Reached target FC %.2f%% — switching to ATPG\n\n", fc);
                    break;
                }
                if ((delta_fc == 0)&&use_rtpg_v3) {
                    pattern_list.pop_back(); 
                    printf("No Faults Detected by Pattern.\n");
                    continue;
                }
                if ((delta_fc <= delta_fc_threshold)&&use_rtpg_v3) {
                    printf("ΔFC = %.2f%% is below threshold %.2f%%.  Exiting...\n", delta_fc, delta_fc_threshold);
                    break;
                }
            }
        }
        infile.close();
        fault_list = remaining_faults;
        // initial_faults = fault_list;
        prev_fc = 100.0f * (initial_faults.size() - fault_list.size() - undetectable_faults.size()) / initial_faults.size();

    }


    std::vector<std::pair<NSTRUC*, int>> deferred_faults;
    int pattern_count = 0;
    bool end_flag = false;
    while ((!fault_list.empty() || !deferred_faults.empty()) && (!use_rtpg_v3) && (!use_rtpg_v4)) {
        auto fault = fault_list.back();
        if (!fault_list.empty()) {
            fault_list.pop_back();
        }
        if (fault_list.empty() && !deferred_faults.empty()) {
            fault = deferred_faults.back();
            deferred_faults.pop_back();
        }

        int key = fault.first->num * 2 + fault.second;
        if ((!fault_list.empty()) && use_rfl && rfl_keys.count(key) == 0 && !rfl_keys.empty()) {
            deferred_faults.push_back(fault);
            printf("   Fault %d@%d not in RFL, skipping...\n", fault.first->num, fault.second);
            continue;
        }

        if (use_rfl && !end_flag) {
            rfl_keys.erase(key);
            // printf("RFL keys remaining: %zu\n", rfl_keys.size());
            if (rfl_keys.empty()) {
                printf("RFL phase complete.  Switching to non-RFL Faults...\n\n");
                end_flag = true;
                continue;
            }
        }


// READ ../ckts_all/c6288.ckt
// TPG PODEM PFS my_patterns.tp -fl rfl



        std::vector<int> pattern;
        bool success = false;

        // Call ATPG
        // printf("DEBUG: Running ATPG for node %d stuck-at-%d\n", fault.first ? fault.first->num : -1, fault.second);
        // printf("   Running ATPG... (%d@%d)\n", fault.first->num, fault.second);
        if (use_scoap) {
            int difficulty = get_fault_difficulty(fault.first, fault.second);
            const char* cc_bit = (fault.second == 0) ? "1" : "0";
            printf("   Running ATPG... (%d@%d) (CC%d + CO = %d)\n", fault.first->num, fault.second, 1 - fault.second, difficulty);
            // printf("   Running ATPG... (%d@%d) -- CC0=%d, CC1=%d, CO=%d, CC+CO=%d\n", fault.first->num, fault.second, fault.first->cc0, fault.first->cc1, fault.first->co, difficulty);

        } else {
            printf("   Running ATPG... (%d@%d)\n", fault.first->num, fault.second);
        }     
        success = run_atpg(fault.first, fault.second, pattern, use_podem);
        if (!success) {
            // printf("      No Test Pattern Generated for Fault %d@%d\n\n", fault.first->num, fault.second);
            delta_fc = 0;
            printf("      No Test Pattern Generated \n");
            printf("ATPG #%d → FC = %.2f%%, ΔFC: %.2f%% (%zu faults remaining)\n\n", ++pattern_count, current_fc, delta_fc, fault_list.size());
            undetectable_faults.push_back(fault);
            continue;
        }

// READ ../ckts_all/c499.ckt
// TPG PODEM DFS my_patterns.tp -rtp v1 30

        if (use_tvc) {
            printf("   Optimizing pattern with TVC...\n");
            for (int i = 0; i < pattern.size(); ++i) {
                if (pattern[i] == 2) { // Found an X
                    // Try setting to 0
                    pattern[i] = 0;
                    std::vector<std::pair<NSTRUC*, int>> detected0;
                    run_fs(pattern, use_dfs, detected0);

                    // Try setting to 1
                    pattern[i] = 1;
                    std::vector<std::pair<NSTRUC*, int>> detected1;
                    run_fs(pattern, use_dfs, detected1);

                    // Choose better
                    if (detected1.size() > detected0.size()) {
                        pattern[i] = 1; // keep as 1
                    } else {
                        pattern[i] = 0; // keep as 0
                    }
                    printf("      Optimized pattern: ");
                    for (int b : pattern) {
                        printf("%c", b == 0 ? '0' : b == 1 ? '1' : 'x');
                    }
                    printf("\n");
                }
            }
        }


        pattern_list.push_back(pattern);
        // Fault simulation
        std::vector<std::pair<NSTRUC*, int>> detected;
        run_fs(pattern, use_dfs, detected);

        // Remove detected faults from the list
        for (auto& f : detected) {
            fault_list.erase(
                std::remove(fault_list.begin(), fault_list.end(), f),
                fault_list.end()
            );
            int fkey = f.first->num * 2 + f.second;
            if (rfl_keys.count(fkey)) {
                rfl_keys.erase(fkey);
            }
        }
        // Calculate fault coverage delta
        total_faults = initial_faults.size();
        current_fc = 100.0f * (total_faults - fault_list.size() - undetectable_faults.size()) / total_faults;
        delta_fc = current_fc - prev_fc;
        prev_fc = current_fc;
        printf("ATPG #%d → FC = %.2f%%, ΔFC: %.2f%% (%zu faults remaining)\n\n", ++pattern_count, current_fc, delta_fc, fault_list.size());

        // printf("DEBUG: Checking if ΔFC %.2f < threshold %.2f\n", delta_fc, delta_fc_threshold);

        // Exit ATPG if delta FC too small
        if (use_rtpg_v2 && delta_fc < delta_fc_threshold) {
            printf("ΔFC = %.2f%% is below threshold %.2f%%.  Exiting...\n", delta_fc, delta_fc_threshold);
            break;
        }
    }
    int detected_faults = initial_faults.size() - fault_list.size() - undetectable_faults.size();
    float final_fc = 100.0f * detected_faults / initial_faults.size();
    printf("Final Fault Coverage = %.2f%%\n", final_fc);
    printf("Detected: %d, Undetectable: %zu, Remaining: %zu\n",
       detected_faults, undetectable_faults.size(), fault_list.size());


    // Write the generated patterns to a .tp file
    // printf("DEBUG: Writing %zu patterns to output file\n", pattern_list.size());
    std::ofstream fout(output_file);
    for (const auto& p : pattern_list) {
        for (int i = 0; i < p.size(); ++i) {
            if (p[i] == 0) fout << "0";
            else if (p[i] == 1) fout << "1";
            else fout << "x";
            if (i < p.size() - 1) fout << ",";
        }
        fout << '\n';
    }
    fout.close();       
    // printf("TPG complete. %zu test patterns written to tpg_output.tp\n", pattern_list.size());
    printf("TPG complete. %zu test patterns written to %s\n", pattern_list.size(), output_file);
    auto tpg_end = std::chrono::high_resolution_clock::now();
    double elapsed_sec = std::chrono::duration<double>(tpg_end - tpg_start).count();
    printf("TPG time elapsed: %.3f seconds\n", elapsed_sec); 

}

// BASELINE:
// READ ../auto-tests-phase3/ckts/c17.ckt
// TPG PODEM DFS my_patterns.tp
// TPG DALG DFS my_patterns.tp

// RTPG:
// READ ../ckts_all/c499.ckt
// TPG DALG PFS my_patterns.tp
// TPG PODEM PFS my_patterns.tp
// TPG PODEM DFS my_patterns.tp
// TPG PODEM DFS my_patterns.tp -rtp v1 99
// TPG PODEM PFS my_patterns.tp -rtp v2 0.02
// TPG PODEM PFS my_patterns.tp -rtp v3 0.02
// TPG PODEM PFS my_patterns.tp -rtp v4 0.1

// DTPFC my_patterns.tp 1 podem_report_499_DFS.out
// DTPFC my_patterns.tp 1 podem_report_499_PFS.out


// D-FRONTIER:
// READ ../ckts_all/c1355.ckt
// TPG PODEM PFS my_patterns.tp -df nl
// TPG PODEM PFS my_patterns.tp -df nh
// TPG PODEM PFS my_patterns.tp -df lh
// TPG PODEM PFS my_patterns.tp -df cc

// J-FRONTIER:
// READ ../ckts_all/c432.ckt
// TPG PODEM PFS my_patterns.tp -jf v0
// TPG PODEM PFS my_patterns.tp

// FAULT ORDER
// READ ../ckts_all/c6288.ckt
// TPG PODEM PFS my_patterns.tp -fl rfl
// TPG PODEM PFS my_patterns.tp -fl scoap EZ
// TPG PODEM PFS my_patterns.tp -fl scoap HD

// TEST VOLUME COMPRESSION:
// READ ../ckts_all/c6288.ckt
// TPG PODEM PFS my_patterns.tp -tvc

// DTPFC:
// READ ../ckts_all/c1355.ckt
// TPG PODEM PFS my_patterns.tp
// DTPFC my_patterns.tp 1 dalg_report_1908.out
// DTPFC my_patterns.tp 1 podem_report_1908.out
// DTPFC my_patterns.tp 1 podem_report_880.out
// DTPFC my_patterns.tp 1 podem_report_1908_rtpgv1.out
// DTPFC my_patterns.tp 1 podem_report_1908_rtpgv2.out
// DTPFC my_patterns.tp 1 podem_report_1908_rtpgv3.out
// DTPFC my_patterns.tp 1 podem_report_1908_rtpgv4.out
// DTPFC my_patterns.tp 1 podem_report_1908_jf.out
// DTPFC my_patterns.tp 1 podem_report_1908_fl_rfl.out
// DTPFC my_patterns.tp 1 podem_report_1908_fl_scoap_ez.out
// DTPFC my_patterns.tp 1 podem_report_1908_fl_scoap_hd.out
// DTPFC my_patterns.tp 1 podem_report_1908_tvc.out
// DTPFC my_patterns.tp 1 podem_report_880_tvc.out
// DTPFC my_patterns.tp 1 podem_report_1908_DFS.out
// DTPFC my_patterns.tp 1 podem_report_6288_DFS.out
// DTPFC my_patterns.tp 1 podem_report_6288_PFS.out

// RANDOM SEED:
// ./simulator 1234



// DTPFC my_patterns.tp 1 dtpfc_report_499.out
