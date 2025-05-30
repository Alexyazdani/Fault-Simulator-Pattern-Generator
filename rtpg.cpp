#include "rtpg.h"
#include "circuit.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>

extern char *cp; //Input arguments are stored here
extern int Npi;
extern NSTRUC **Pinput;
extern int seed;

void rtpg() {
    int tp_count;
    char mode;
    std::string tp_file;
    std::vector<int> PI_nodes;
    
    for (int i = 0; i < Npi; ++i) {
        PI_nodes.push_back(Pinput[i]->num);
    }

    if (cp == nullptr) {
        std::cerr << "Error: No arguments provided for RTPG.\n";
        return;
    }

    std::istringstream iss(cp);
    if (!(iss >> tp_count >> mode >> tp_file)) {
        std::cerr << "Error: Invalid arguments. Usage: RTPG <count> <mode> <filename>\n";
        return;
    }

    if (mode != 'b' && mode != 't') {
        std::cerr << "Error: Invalid mode. Use 'b' for binary or 't' for ternary.\n";
        return;
    }

    std::ofstream outfile(tp_file);
    if (!outfile) {
        std::cerr << "Error: Cannot open file " << tp_file << " for writing.\n";
        return;
    }

    // Write PI node IDs
    for (size_t i = 0; i < PI_nodes.size(); i++) {
        outfile << PI_nodes[i];
        if (i < PI_nodes.size() - 1) outfile << ",";
    }
    outfile << "\n";

    // Generate test patterns
    // srand(time(0));
    // unsigned int seed = 658;
    // srand(seed);
    printf("Using seed: %u\n", seed);
    for (int i = 0; i < tp_count; i++) {
        for (size_t j = 0; j < PI_nodes.size(); j++) {
            if (mode == 'b') {
                outfile << (rand() % 2);
            } else {
                int rnd = rand() % 3;
                if (rnd == 0) outfile << '0';
                else if (rnd == 1) outfile << '1';
                else outfile << 'x';
            }
            if (j < PI_nodes.size() - 1) outfile << ",";
        }
        outfile << "\n";
    }

    outfile.close();
}