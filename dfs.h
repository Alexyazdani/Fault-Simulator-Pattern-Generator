#ifndef DFS_H
#define DFS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <sstream>
#include <string>
#include "circuit.h"
#include "logicsim.h"
#include "levelizer.h"

using namespace std;

// Constants and Global Variables
extern int Nnodes;
extern int Npo;

// External Global Variables
extern NSTRUC *Node;
extern NSTRUC **Poutput;
extern char *cp; // Input argument pointer

// Function Declarations
void dfs();
void dfs_single(const vector<int>& input_pattern, set<string>& detected_faults);

#endif // DFS_H
