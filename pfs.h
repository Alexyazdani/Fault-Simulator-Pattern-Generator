#ifndef PFS_H
#define PFS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <set>
#include <unordered_set>
#include "circuit.h"
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
void pfs();                  // Main function
//void parallel_node_value();   // LOGICSIM function (assumed external)

#endif // PFS_H
