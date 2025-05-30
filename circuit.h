/*
Alexander Yazdani
USC Spring 2025
EE658
*/

#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <ctype.h>
#include <cstring>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <list>
#include <map>

#define MAXLINE 4096
#define MAXNAME 1024

#define Upcase(x) ((isalpha(x) && islower(x))? toupper(x) : (x))
#define Lowcase(x) ((isalpha(x) && isupper(x))? tolower(x) : (x))

enum e_com {READ, PC, HELP, QUIT, LEV, LOGICSIM, RTPG, PFS, RFL, DFS, TPFC, SCOAP, DALG, PODEM};  
enum e_state {EXEC, CKTLD};         
enum e_ntype {GATE, PI, FB, PO};    
enum e_gtype {IPT, BRCH, XOR, OR, NOR, NOT, NAND, AND, XNOR, BUFFER};  

struct cmdstruc {
   char name[MAXNAME];        
   void (*fptr)();            
   enum e_state state;        
};

enum LogicVal {
    L_0 = 0,  // Logic 0
    L_1 = 1,  // Logic 1
    L_X = 2,  // Unknown
    L_D = 3,  // 1 (good) / 0 (faulty)
    L_DB = 4  // 0 (good) / 1 (faulty)
};

inline const char* logicval_to_str(LogicVal v) {
    switch (v) {
        case L_0: return "0";
        case L_1: return "1";
        case L_X: return "x";
        case L_D: return "D";
        case L_DB: return "D'";
        default:  return "?";
    }
}
typedef struct n_struc {
    unsigned indx;             
    unsigned num;              
    enum e_ntype ntype;
    enum e_gtype type;         
    unsigned fin;              
    unsigned fout;             
    struct n_struc **unodes;   
    struct n_struc **dnodes;   
    int level;                 
    int value;
    LogicVal d_value;
    int ormask = 0;
    int andmask = -1;
    int node_value;
    int true_value;     
    int cc0, cc1, co;    
    bool visited;        
    bool force_d_value;    
    bool decision_tag = false;
    int index;
    bool changed = false;
    bool set_objective = false;
    int assign_level;  // Used for D-algorithm recursion tracking
    int eval_value; 
} NSTRUC;                     

#define NUMFUNCS 16

extern struct cmdstruc command[NUMFUNCS];

// Global variables
extern enum e_state Gstate;    
extern NSTRUC *Node;           
extern NSTRUC **Pinput;        
extern NSTRUC **Poutput;       
extern int Nnodes;             
extern int Npi;                
extern int Npo;                
extern int Done;               
extern char *cp;
extern char inFile[MAXLINE];
extern std::string inp_name;

std::string gname(int tp);
void help();
void quit();
void pc();
void parallel_node_value();
LogicVal evaluate_d_gate(NSTRUC *node);
LogicVal d_and(LogicVal a, LogicVal b);
LogicVal d_or(LogicVal a, LogicVal b);
LogicVal d_not(LogicVal a);
LogicVal d_xor(LogicVal a, LogicVal b);
extern NSTRUC *fault_node;
bool inject_fault(int fault_node_num, int fault_val);
void reset_d_values();
void reset_for_backtrack(int fault_node, int fault_val);

const char* nodetype_to_str(int type);

extern std::list<int> to_imply;             /* List to imply*/
extern std::map<int, int> events;
#endif
