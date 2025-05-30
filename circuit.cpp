/*
Alexander Yazdani
USC Spring 2025
EE658
*/

#include "circuit.h"
#include "parser.h"
#include "levelizer.h"
#include "logicsim.h"
#include "rtpg.h"
#include "pfs.h"
#include "rfl.h"
#include "dfs.h"
#include "tpfc.h"
#include "scoap.h"
#include "dalg.h"
#include "podem.h"
#include "atpg.h"
#include "tpg.h"
#include "dtpfc.h"

enum e_state Gstate = EXEC;     
NSTRUC *Node = nullptr;         
NSTRUC **Pinput = nullptr;      
NSTRUC **Poutput = nullptr;     
int Nnodes = 0;                 
int Npi = 0;                    
int Npo = 0;                    
int Done = 0;                   
char *cp = nullptr;
char inFile[MAXLINE];

std::list<int> to_imply;
std::map<int, int> events;
int Nlevels = 0; 

struct cmdstruc command[NUMFUNCS] = {
    {"READ", cread, EXEC},
    {"PC", pc, CKTLD},
    {"HELP", help, EXEC},
    {"QUIT", quit, EXEC},
    {"LEV", lev, CKTLD},
    {"LOGICSIM", logicsim, CKTLD},
    {"RTPG", rtpg, EXEC}, 
    {"RFL", rfl, CKTLD},
    {"PFS", pfs, CKTLD},
    {"DFS", dfs, CKTLD},
    {"TPFC", tpfc, CKTLD},
    {"SCOAP", scoap, CKTLD},
    {"DALG", dalg, CKTLD},
    {"PODEM", podem, CKTLD},
    {"TPG", tpg, CKTLD},
    {"DTPFC", dtpfc, CKTLD}
};

/*-----------------------------------------------------------------------
input: nothing
output: nothing
called by: main
description:
    The routine prints out the circuit description from previous READ command.
-----------------------------------------------------------------------*/
void pc(){
    int i, j;
    NSTRUC *np;
    std::string gname(int);
   
    printf(" Node   Type \tIn     \t\t\tOut    \n");
    printf("------ ------\t-------\t\t\t-------\n");
    for(i = 0; i<Nnodes; i++) {
        np = &Node[i];
        printf("\t\t\t\t\t");
        for(j = 0; j<np->fout; j++) printf("%d ",np->dnodes[j]->num);
        printf("\r%5d  %s\t", np->num, gname(np->type).c_str());
        for(j = 0; j<np->fin; j++) printf("%d ",np->unodes[j]->num);
        printf("\n");
    }
    printf("Primary inputs:  ");
    for(i = 0; i<Npi; i++) printf("%d ",Pinput[i]->num);
    printf("\n");
    printf("Primary outputs: ");
    for(i = 0; i<Npo; i++) printf("%d ",Poutput[i]->num);
    printf("\n\n");
    printf("Number of nodes = %d\n", Nnodes);
    printf("Number of primary inputs = %d\n", Npi);
    printf("Number of primary outputs = %d\n", Npo);
}


/*-----------------------------------------------------------------------
input: nothing
output: nothing
called by: main 
description:
    The routine prints ot help inormation for each command.
-----------------------------------------------------------------------*/
void help(){
    printf("READ filename - ");
    printf("read in circuit file and creat all data structures\n");
    printf("PC - ");
    printf("print circuit information\n");
    printf("HELP - ");
    printf("print this help information\n");
    printf("QUIT - ");
    printf("stop and exit\n");
    printf("LEV out_filename ");
    printf("levelize circuit\n");
    printf("LOGICSIM - ");
    printf("run logic simulation on the circuit\n");
    printf("RTPG - ");
    printf("perform random test pattern generation\n");
    printf("RFL - ");
    printf("generate reduced fault list\n");
    printf("DFS - ");
    printf("perform deductive fault simulation\n");
    printf("PFS - ");
    printf("perform parallel fault simulation\n");
    printf("TPFC - ");
    printf("perform test pattern fault coverage\n");
    printf("SCOAP - ");
    printf("perform scoap\n");
    printf("DALG - ");
    printf("perform d-algorithm\n");
    printf("PODEM - ");
    printf("perform podem algorithm\n");
    printf("TPG - ");
    printf("perform test pattern generation\n");
}

/*-----------------------------------------------------------------------
input: nothing
output: nothing
called by: main 
description:
    Sets Done to 1, which terminates the program.
-----------------------------------------------------------------------*/
void quit(){
    Done = 1;
}

/*-----------------------------------------------------------------------
input: gate type
output: string of the gate type
called by: pc
description:
    The routine receives an integer gate type and returns the gate type in
    a character string.
-----------------------------------------------------------------------*/
std::string gname(int tp){
    switch(tp) {
        case 0: return("PI");
        case 1: return("BRANCH");
        case 2: return("XOR");
        case 3: return("OR");
        case 4: return("NOR");
        case 5: return("NOT");
        case 6: return("NAND");
        case 7: return("AND");
        case 8: return("XNOR");
        case 9: return("BUFFER");
    }
    return "";
}

void parallel_node_value () {
   
    int i, j, prev_value;
    NSTRUC *np;
    int max_node = 0;
    for (int node = 0; node < Nnodes; node++){
          np = &Node[node];
          if (np->level > max_node)
             max_node = np->level;
 
       }
 
    int current_level = 0;	
    bool checkflag;
    
    while(current_level <= max_node) {
 
       for (int node = 0; node<Nnodes; node++){
       np = &Node[node];    //this stores the first node in the queue
       if (np->level == current_level){
       prev_value = np->node_value;   //this stores the previous node value 
       switch(np->type) {
          case 0:  // PI
                  np->node_value = ((np->node_value) & (np->andmask)) | np->ormask;
         break;   
 
          case 1:  // BRANCH 
             np->node_value = ((np->unodes[0]->node_value) & (np->andmask)) | (np->ormask);
             break;
 
          case 2:  // XOR gate
             np->node_value = 0;  
             for (int f_in =0; f_in <np->fin; f_in++){
                np->node_value = np->node_value ^ np->unodes[f_in]->node_value;
             }
             np->node_value = ((np->node_value) & (np->andmask)) | (np->ormask);
 
             break; 
          case 3:  // OR gate
             np->node_value = 0;  
             for (j = 0; j < np->fin; j++) {
                np->node_value = (np->node_value) | (np->unodes[j]->node_value);
             } 
             np->node_value = ((np->node_value) & (np->andmask)) | (np->ormask);
             break;
          case 4:  // NOR gate
             np->node_value = 0;    
             for (j = 0; j < np->fin; j++) {
                np->node_value = (np->node_value) | (np->unodes[j]->node_value);
             }
             np->node_value = ~ (np->node_value);
             np->node_value = ((np->node_value) & (np->andmask)) | (np->ormask);         
             break; 
          case 5: // NOT gate
             np->node_value = ~ np->unodes[0]->node_value;
             np->node_value = ((np->node_value) & (np->andmask)) | (np->ormask); 
             break; 
          case 6:  // NAND gate
             np->node_value = -1;    
             for (j = 0; j < np->fin; j++) {
                np->node_value = (np->node_value) & (np->unodes[j]->node_value);
             }
             np->node_value = ~ (np->node_value);
             np->node_value = ((np->node_value) & (np->andmask)) | (np->ormask);             
             break; 
          case 7:  // AND gate
             np->node_value = -1;    
             for (j = 0; j < np->fin; j++) {
                np->node_value = (np->node_value) & (np->unodes[j]->node_value);
             }
             np->node_value = ((np->node_value) & (np->andmask)) | (np->ormask);
 
             break;
          case 8:  // XNOR gate
             np->node_value = 0;  
             for (int f_in =0; f_in <np->fin; f_in++){
                np->node_value = np->node_value ^ np->unodes[f_in]->node_value;
             }
             np->node_value = ~ (np->node_value);
             np->node_value = ((np->node_value) & (np->andmask)) | (np->ormask);
 
             break; 
          case 9:  // Buffer 
             np->node_value = ((np->unodes[0]->node_value) & (np->andmask)) | (np->ormask);
             break;
       }
       }
     // removes the first element as it has been evaluated
    }
    current_level = current_level + 1;
    }
 }
