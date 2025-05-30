#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <map>
#include <string>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <chrono>

#include "circuit.h"
#include "levelizer.h"

//Gate type 
#define GATE_PI 0
#define GATE_BRANCH 1
#define GATE_XOR 2
#define GATE_OR 3
#define GATE_NOR 4
#define GATE_NOT 5
#define GATE_NAND 6
#define GATE_AND 7
#define GATE_XNOR 8
#define GATE_BUF 9

//Logic values 
#define LOGIC_X -1
#define LOGIC_0 0
#define LOGIC_1 1
#define LOGIC_D 2
#define LOGIC_DBAR 3
#define LOGIC_UNSET 4

//Fault Type 
#define NOFAULT   -1
#define FAULT_SA0 0
#define FAULT_SA1 1

using namespace std;
using sec = std::chrono::duration<double>; 
using ms = std::chrono::duration<double, std::milli>; 

deque<int> Dfront;
vector<int> Jfront;
vector<int> imply;
vector< vector<int> > test_vectors;

int Dalg_count;

pair<int,int> check_fault;
clock_t dalg_recursion_tStart;
int dalgo_count = 0;

bool unassign(int level) {  
    for(int k=0;k<Nnodes;k++) { 
       if(Node[k].assign_level>level ) {
          Node[k].value=LOGIC_X; 
          Node[k].assign_level=-1;
       } 
    } 
    return true;
 }

 int controlling_value(int node_index){
    switch(Node[node_index].dnodes[0]->type){
        case NAND: return 0;
        case AND: return 0;
        case OR: return 1;
        case NOR: return 1;
        case XOR:
            if(Node[node_index].cc0 < Node[node_index].cc1) return 0;
            else return 1;
        case XNOR:
            if(Node[node_index].cc0 < Node[node_index].cc1) return 0;
            else return 1;
    }
}

 bool check_DFrontier (int node){
   
    NSTRUC *nodeptr; 
    nodeptr =&Node[node];
    int dNum = 0; 
    int dbNum = 0, oneNum = 0, zeroNum = 0;
    for(int j = 0;j<(nodeptr->fin); j++ ){
       if(nodeptr->unodes[j]->value == LOGIC_D) dNum++;
       else if(nodeptr->unodes[j]->value == LOGIC_DBAR) dbNum++;
       else if(nodeptr->unodes[j]->value == LOGIC_1) oneNum++;
       else if(nodeptr->unodes[j]->value == LOGIC_0) zeroNum++;
    }
    if( (dNum == 0 && dbNum == 0)|| nodeptr->value!=LOGIC_X ||(dNum > 0 && dbNum > 0) || (oneNum > 0 && (nodeptr->type == GATE_OR || nodeptr->type == GATE_NOR || nodeptr->type == GATE_NOT || nodeptr->type == GATE_BRANCH))
       || (zeroNum > 0 && (nodeptr->type == GATE_NAND || nodeptr->type == GATE_AND || nodeptr->type == GATE_NOT || nodeptr->type == GATE_BRANCH )))
    { 
        return false;
    }
    else 
    { 
        return true; 
    }
    
 }

 int get_DFrontier(){
   
    vector<int> temp; 
    temp.clear();
    Dfront.clear();
    temp.push_back((int)(check_fault.first));
 
       for(int i = 0; i < temp.size(); i++){
          NSTRUC *nodeptr =&Node[temp[i]];
          if(nodeptr->fout > 0){
             for( int k = 0;k < nodeptr->fout; k++){
                if  ( check_DFrontier(nodeptr->dnodes[k]->indx) ) { 
                      Dfront.push_back(nodeptr->dnodes[k]->indx);
                }
                else if ( nodeptr->dnodes[k]->value == LOGIC_D || nodeptr->dnodes[k]->value == LOGIC_DBAR ) {
                   temp.push_back(nodeptr->dnodes[k]->indx);
                }
             }
          }
       }
       return 0;
 }

 bool imply_and_check(){
	
	for(int k=0; k< imply.size(); k++) {
	    NSTRUC *nodeptr =&Node[imply[k]];
	    if(nodeptr->value != LOGIC_D && nodeptr->value != LOGIC_DBAR) {
	    
            // Backward implication 
		    if(nodeptr->type == GATE_PI) return true;
        
        //Branch 

        else if( nodeptr->type == GATE_BRANCH) {		
            for(int j = 0; j<(nodeptr->fin); j++) {
                if(nodeptr->value == LOGIC_0) { 
                    if(nodeptr->unodes[j]->value == LOGIC_X) { 
                        nodeptr->unodes[j]->value = LOGIC_0;   
                        imply.push_back(nodeptr->unodes[j]->indx);
                    } 
                    else if (nodeptr->unodes[j]->value!=LOGIC_0 ) {
                        return false;
                    }
                }
                if(nodeptr->value == LOGIC_1) { 
                    if(nodeptr->unodes[j]->value == LOGIC_X) { 
                        nodeptr->unodes[j]->value = LOGIC_1;   
                        imply.push_back(nodeptr->unodes[j]->indx);
                    } 
                    else if (nodeptr->unodes[j]->value!=LOGIC_1 ) {
                    return false;
                    }
                }
            } 
         
         }

        // Forward Propagation

        //XOR
        // J Frontier : 1. Atleast two inputs are unknown  2. Output is to be implied (No control value of inputs)

        else if (nodeptr->type == GATE_XOR) { 
            int num_input_X = 0; 
            int oneXind = 0; 
            int implyvalue = nodeptr->value; 
            for(int j = 0 ; j<(nodeptr->fin) ; j++) { 
                if(nodeptr->unodes[j]->value == LOGIC_1 || nodeptr->unodes[j]->value == LOGIC_0) {
                    implyvalue ^= nodeptr->unodes[j]->value;
                } 
                else if (nodeptr->unodes[j]->value == LOGIC_D || nodeptr->unodes[j]->value == LOGIC_DBAR) {
                    return false;
                } 
                else {
                    oneXind = j;
                    num_input_X++; 
                }
            }
            
            if(num_input_X == 1) { 
               nodeptr->unodes[oneXind]->value =  implyvalue;   
               imply.push_back(nodeptr->unodes[oneXind]->indx);   
            } 
            else {
                Jfront.push_back(nodeptr->indx);
            } 
         } 

        //XNOR 

        else if (nodeptr->type == GATE_XNOR) { 
        int num_input_X = 0; 
        int oneXind = 0; 
        int implyvalue =! nodeptr->value; 
        for(int j = 0 ; j<(nodeptr->fin) ; j++) { 
            if(nodeptr->unodes[j]->value == LOGIC_1 || nodeptr->unodes[j]->value == LOGIC_0) {
                implyvalue^=nodeptr->unodes[j]->value;
            } 
            else if (nodeptr->unodes[j]->value == LOGIC_D || nodeptr->unodes[j]->value == LOGIC_DBAR) {
                return false;
            } 
            else {
                oneXind = j;
                num_input_X++; 
            }
        }
        if(num_input_X == 1) { 
            nodeptr->unodes[oneXind]->value = !implyvalue;   
            imply.push_back(nodeptr->unodes[oneXind]->indx);   
        } 
        else {
            Jfront.push_back(nodeptr->indx);
        } 
        } 
		
        //OR

        else if  ( nodeptr->type == GATE_OR) {
            if(nodeptr->value == LOGIC_0)  { 
                for(int j = 0; j<(nodeptr->fin) ;j++){
                    if(nodeptr->unodes[j]->value != LOGIC_0 && nodeptr->unodes[j]->value != LOGIC_X) {
                        return false;
                    }
                    else if(nodeptr->unodes[j]->value == LOGIC_X){ 
                        nodeptr->unodes[j]->value = 0;   
                        imply.push_back(nodeptr->unodes[j]->indx);   
                    }
                }   
            }
            if(nodeptr->value == LOGIC_1){ 
                bool has_controlvalue = false; 
                int unknown_num = 0;
                int oneXind = 0; 
                int dNum = 0; 
                int dbNum = 0;
                for(int j = 0;j<(nodeptr->fin);j++) {
                    if(nodeptr->unodes[j]->value == LOGIC_1) { 
                        has_controlvalue = true; 
                        break;
                    } 
                    else if (nodeptr->unodes[j]->value == LOGIC_X) {
                        unknown_num++;  
                        oneXind = j;
                    }   
                    else if (nodeptr->unodes[j]->value == LOGIC_D) { 
                        dNum++; 
                    } 
                    else if (nodeptr->unodes[j]->value == LOGIC_DBAR) { 
                        dbNum++; 
                    }
                }

                if(has_controlvalue == false && unknown_num == 0) {
                    if(dNum == 0 || dbNum == 0) { 
                        return false;
                    }
                }
                else if(has_controlvalue == false && unknown_num == 1) {     
                    if(!(dNum>0 && dbNum>0)) {
                        nodeptr->unodes[oneXind]->value = LOGIC_1;   
                        imply.push_back(nodeptr->unodes[oneXind]->indx); 
                    }
                }              
                else if(has_controlvalue == false && unknown_num>1 && !((dNum>0 && dbNum>0))) { 
                    Jfront.push_back(nodeptr->indx);
                }
            }
        }
		
        //NOR

        else if  ( nodeptr->type == GATE_NOR) {
            if(nodeptr->value == LOGIC_1)  { 
                for(int j=0;j<(nodeptr->fin);j++){
                    if(nodeptr->unodes[j]->value!=LOGIC_0 && nodeptr->unodes[j]->value!=LOGIC_X) {
                        return false;
                    } 
                    else if (nodeptr->unodes[j]->value == LOGIC_X) {
                        nodeptr->unodes[j]->value = LOGIC_0;   
                        imply.push_back(nodeptr->unodes[j]->indx);
                    }
                }   
            }
            
            if(nodeptr->value == LOGIC_0) {
                bool has_controlvalue = false; 
                int unknown_num = 0;
                int oneXind = 0; 
                int dNum = 0; 
                int dbNum = 0;
                for(int j = 0; j<(nodeptr->fin); j++) {
                    if(nodeptr->unodes[j]->value == LOGIC_1) { 
                        has_controlvalue=true; 
                    break;
                    } 
                    else if (nodeptr->unodes[j]->value == LOGIC_X) {
                        unknown_num++;  
                        oneXind = j;
                    } 
                    else if (nodeptr->unodes[j]->value == LOGIC_D) {
                        dNum++;
                    } 
                    else if (nodeptr->unodes[j]->value == LOGIC_DBAR) {
                        dbNum++; 
                    }
                }
                if(has_controlvalue == false && unknown_num == 0) {
                    if(dNum == 0 || dbNum == 0) {
                        return false;
                    }
                }   
                else if (has_controlvalue == false && unknown_num  == 1) {     
                    if(!(dNum>0 && dbNum>0)) {
                        nodeptr->unodes[oneXind]->value = LOGIC_1;   
                        imply.push_back(nodeptr->unodes[oneXind]->indx); 
                    }
                } 
                else if (has_controlvalue == false && unknown_num>1 && !((dNum>0 && dbNum>0))) {
                    Jfront.push_back(nodeptr->indx);
                }
                }
            } 

        //NOT

        else if  ( nodeptr->type == GATE_NOT) {
            if(nodeptr->value == LOGIC_0) {
                if(nodeptr->unodes[0]->value == LOGIC_X) { 
                    nodeptr->unodes[0]->value = LOGIC_1;   
                    imply.push_back(nodeptr->unodes[0]->indx);
                } 
                else if(nodeptr->unodes[0]->value!=LOGIC_1) {
                    return false;
                }
            }
            if (nodeptr->value == LOGIC_1) {
                if (nodeptr->unodes[0]->value == LOGIC_X) {
                    nodeptr->unodes[0]->value = LOGIC_0;
                    imply.push_back(nodeptr->unodes[0]->indx);
                } 
                else if (nodeptr->unodes[0]->value!=LOGIC_0) {
                    return false;
                }
            }
        } 

        //BUFFER
        else if(nodeptr->type == GATE_BUF){
            if(nodeptr->value == LOGIC_0){
                if(nodeptr->unodes[0]->value == LOGIC_X){
                nodeptr->unodes[0]->value = LOGIC_0;
                imply.push_back(nodeptr->unodes[0]->indx);
                } 
                else if(nodeptr->unodes[0]->value!=LOGIC_0){
                return false;
                }
            }
            if(nodeptr->value == LOGIC_1){
                if(nodeptr->unodes[0]->value == LOGIC_X){
                    nodeptr->unodes[0]->value=LOGIC_1;
                    imply.push_back(nodeptr->unodes[0]->indx);
                } 
                else if(nodeptr->unodes[0]->value!=LOGIC_1){
                return false;
                }
            }
        }

        //NAND

        else if  ( nodeptr->type  ==  GATE_NAND) {
            if(nodeptr->value == LOGIC_0)  { 
                for(int j=0;j<(nodeptr->fin);j++){
                    if(nodeptr->unodes[j]->value!=LOGIC_1 && nodeptr->unodes[j]->value!=LOGIC_X) {
                        return false;
                    } 
                    else if (nodeptr->unodes[j]->value == LOGIC_X) {
                        nodeptr->unodes[j]->value = LOGIC_1;
                        imply.push_back(nodeptr->unodes[j]->indx);
                    }
                }   
            }
            if (nodeptr->value == LOGIC_1) {
                bool has_controlvalue=false; 
                int unknown_num=0;
                int oneXind=0; 
                int dNum=0; 
                int dbNum=0;
                for(int j=0;j<(nodeptr->fin);j++) {
                    if(nodeptr->unodes[j]->value == LOGIC_0) { 
                        has_controlvalue=true; 
                        break;
                    } else if (nodeptr->unodes[j]->value == LOGIC_X) {
                        unknown_num++;  
                        oneXind = j;
                    } else if (nodeptr->unodes[j]->value == LOGIC_D) {
                        dNum++; 
                    } else if (nodeptr->unodes[j]->value == LOGIC_DBAR) {
                        dbNum++; 
                    }
                }
                if (has_controlvalue == false && unknown_num == 0) {
                    if(dNum == 0 || dbNum == 0) {
                        return false;
                    }
                } 
                else if (has_controlvalue == false && unknown_num  == 1) {     
                    if(!(dNum>0 && dbNum>0)) {
                        nodeptr->unodes[oneXind]->value =  LOGIC_0;   
                        imply.push_back(nodeptr->unodes[oneXind]->indx); 
                    }
                } 
                else if (has_controlvalue == false && unknown_num>1 && !((dNum>0 && dbNum>0))) {
                    Jfront.push_back(nodeptr->indx);
                }
            }
        }

        //AND
         
         else if  ( nodeptr->type  ==  GATE_AND) {
            if(nodeptr->value == LOGIC_1)  { 
                for(int j=0;j<(nodeptr->fin);j++){
                    if(nodeptr->unodes[j]->value!=LOGIC_1 && nodeptr->unodes[j]->value!=LOGIC_X) {
                        return false;
                    } 
                    else if (nodeptr->unodes[j]->value == LOGIC_X) {
                        nodeptr->unodes[j]->value = LOGIC_1;   
                        imply.push_back(nodeptr->unodes[j]->indx);
                    }
                }   
            }
            if(nodeptr->value == LOGIC_0) { 
                bool has_controlvalue=false; 
                int unknown_num=0;
                int oneXind=0; 
                int dNum=0; 
                int dbNum=0;
                for(int j=0;j<(nodeptr->fin);j++) {
                    if(nodeptr->unodes[j]->value == LOGIC_0) { 
                        has_controlvalue=true; 
                        break;
                    } else if (nodeptr->unodes[j]->value == LOGIC_X) {
                        unknown_num++;  
                        oneXind = j;
                    } else if (nodeptr->unodes[j]->value == LOGIC_D) {
                        dNum++; 
                    } else if (nodeptr->unodes[j]->value == LOGIC_DBAR) {
                        dbNum++; 
                    }
                }
                if(has_controlvalue == false && unknown_num == 0) {
                    if(dNum == 0 || dbNum == 0) { 
                        return false;
                    }
                } 
                else if(has_controlvalue == false && unknown_num  == 1) {     
                    if(!(dNum>0 && dbNum>0)) {
                    nodeptr->unodes[oneXind]->value = LOGIC_0;   
                    imply.push_back(nodeptr->unodes[oneXind]->indx); }
                } 
                else if(has_controlvalue == false && unknown_num>1 && !((dNum>0 && dbNum>0))) { 
                    Jfront.push_back(nodeptr->indx);
                }
            }
        } 
	}


	// Forward Imply 							 
    
    for(int j=0;j<(nodeptr->fout);j++){
        NSTRUC *nodeptr_out;		
        nodeptr_out = &Node[nodeptr->dnodes[j]->indx];	
        if(nodeptr->dnodes[j]->num  ==  (check_fault.first) ) continue;
        int Xnum=0;  
        int dNum=0;
        int dbNum=0;
        int oneNum=0;
        int zeroNum=0;
        int expect_outvalue= 0;
        for(int j=0;j<(nodeptr_out->fin);j++) {
            if      (nodeptr_out->unodes[j]->value == LOGIC_X) Xnum++;
            else if (nodeptr_out->unodes[j]->value == LOGIC_D) dNum++;
            else if (nodeptr_out->unodes[j]->value == LOGIC_DBAR) dbNum++;
            else if (nodeptr_out->unodes[j]->value == LOGIC_1) oneNum++; 
            else if (nodeptr_out->unodes[j]->value == LOGIC_0) zeroNum++;
        }

        //BRANCH

        if(nodeptr_out->type == GATE_BRANCH){
            if(nodeptr->value == LOGIC_0) { 
                if(nodeptr_out->value == LOGIC_X) {
                    nodeptr_out->value = LOGIC_0;
                    imply.push_back(nodeptr_out->indx);
                } 
                else if (nodeptr_out->num!=(int)(check_fault.first) && !(nodeptr_out->value == LOGIC_0) && ! (nodeptr_out->value == LOGIC_DBAR)) {
                    return false;
                }
            }
            if(nodeptr->value == LOGIC_1) {
                if(nodeptr_out->value == LOGIC_X) {
                    nodeptr_out->value = LOGIC_1;
                    imply.push_back(nodeptr_out->indx);
                } 
                else if (nodeptr_out->num!=(int)(check_fault.first) &&!(nodeptr_out->value == LOGIC_1) && ! (nodeptr_out->value == LOGIC_D)) {
                    return false;
                }
            }
            if(nodeptr->value == LOGIC_D || nodeptr->value == LOGIC_DBAR ) {
                if(nodeptr_out->value == LOGIC_X) {
                    nodeptr_out->value = nodeptr->value;   
                    imply.push_back(nodeptr_out->indx);
                }
            }
        }

        //NOT

        else if(nodeptr_out->type == GATE_NOT){
            if(nodeptr->value == LOGIC_0) {
                if(nodeptr_out->value == LOGIC_X) {
                    nodeptr_out->value = LOGIC_1;   
                    imply.push_back(nodeptr_out->indx);
                } 
                else if (nodeptr_out->num!=(int)(check_fault.first) && !(nodeptr_out->value == LOGIC_1)  && ! (nodeptr_out->value == LOGIC_D) ) {
                    return false;
                }
            }
            if(nodeptr->value == LOGIC_1) { 
                if(nodeptr_out->value == LOGIC_X) { 
                    nodeptr_out->value = LOGIC_0;   
                    imply.push_back(nodeptr_out->indx);
                } 
                else if (nodeptr_out->num!=(int)(check_fault.first) && !(nodeptr_out->value == LOGIC_0) && ! (nodeptr_out->value == LOGIC_DBAR)) {
                    return false;
                }
            } 
            if(nodeptr->value == LOGIC_D) { 
                if(nodeptr_out->value == LOGIC_X) { 
                    nodeptr_out->value = LOGIC_DBAR;   
                    imply.push_back(nodeptr_out->indx);
                }       
            }
            if(nodeptr->value == LOGIC_DBAR) { 
                if(nodeptr_out->value == LOGIC_X) {
                    nodeptr_out->value = LOGIC_D;   
                    imply.push_back(nodeptr_out->indx);
                }
            }
        }

         //BUF

        else if(nodeptr_out->type  ==  GATE_BUF){
            if(nodeptr->value  ==  LOGIC_1) {
                if(nodeptr_out->value  ==  LOGIC_X) {
                    nodeptr_out->value = LOGIC_1;
                    imply.push_back(nodeptr_out->indx);
                } 
                else if(nodeptr_out->num != (int)(check_fault.first) && !(nodeptr_out->value  ==  LOGIC_1) && !(nodeptr_out->value  ==  LOGIC_D)) {
                    return false;
                }
            }
            if(nodeptr->value  ==  LOGIC_0) {
                if(nodeptr_out->value  ==  LOGIC_X) {
                    nodeptr_out->value = LOGIC_0;
                    imply.push_back(nodeptr_out->indx);
                } 
                else if(nodeptr_out->num != (int)(check_fault.first) && !(nodeptr_out->value  ==  LOGIC_0) && !(nodeptr_out->value  ==  LOGIC_DBAR)) {
                    return false;
                    }
                }
                if(nodeptr->value  ==  LOGIC_D) {
                    if(nodeptr_out->value  ==  LOGIC_X) {
                        nodeptr_out->value = LOGIC_D;
                        imply.push_back(nodeptr_out->indx);
                    }
                }
                if(nodeptr->value  ==  LOGIC_DBAR) {
                    if(nodeptr_out->value  ==  LOGIC_X) {
                        nodeptr_out->value = LOGIC_DBAR;
                        imply.push_back(nodeptr_out->indx);
                    }
                }
            }

        //XOR 

        else if(nodeptr_out->type == GATE_XOR){
        
        // Try to Imply 

            if(nodeptr_out->value  == LOGIC_X && Xnum == 0 ) { 
                if(dNum%2 == 0 && dbNum%2 == 0) {
                    if(oneNum%2 == 1) {
                         nodeptr_out->value=LOGIC_1;
                    } 
                    else {
                        nodeptr_out->value=LOGIC_0;
                    } 
                    imply.push_back(nodeptr_out->indx);
                } 
                else if (dNum == dbNum && oneNum%2 == 1) {
                    nodeptr_out->value=LOGIC_0; 
                    imply.push_back(nodeptr_out->indx);
                } 
                else if(dNum == dbNum && oneNum%2 == 0) {
                    nodeptr_out->value=LOGIC_1; 
                    imply.push_back(nodeptr_out->indx);
                }
            }
            
            // Check Implication 

            else if(nodeptr_out->value == LOGIC_1 && Xnum == 0) { 
                if(dNum%2 == 0 && dbNum%2 == 0) {
                    if(oneNum%2 == 1) {
                        if(nodeptr_out->value == LOGIC_0) {
                            return false;
                        }
                    }
                } 
                else if(dNum == dbNum && oneNum%2 == 0) { 
                    if(nodeptr_out->value == LOGIC_0) {
                        return false;
                    }
                }
            } 
            else if(nodeptr_out->value == LOGIC_0 && Xnum == 0) { 
                if(dNum%2 == 0 && dbNum%2 == 0) {
                    if(oneNum%2 == 0) {
                        if(nodeptr_out->value == LOGIC_1) {
                            return false;
                        }
                    }
                } 
                else if(dNum == dbNum && oneNum%2 == 1) {
                    if(nodeptr_out->value == LOGIC_1) {
                        return false;
                    }
                }
            }
        }

        //XNOR

        else if(nodeptr_out->type == GATE_XNOR) {
        
        // Try to Imply 

        if(nodeptr_out->value == LOGIC_X && Xnum == 0) { 
            if(dNum % 2 == 0 && dbNum % 2 == 0) {
                if(oneNum % 2 == 1) {
                    nodeptr_out->value = LOGIC_0;
                } 
                else {
                    nodeptr_out->value = LOGIC_1;
                } 
                imply.push_back(nodeptr_out->indx);
            } 
            else if (dNum == dbNum && oneNum % 2 == 1) {
                nodeptr_out->value = LOGIC_1; 
                imply.push_back(nodeptr_out->indx);
            } 
            else if(dNum == dbNum && oneNum % 2 == 0) {
                nodeptr_out->value = LOGIC_0; 
                imply.push_back(nodeptr_out->indx);
            }
        }
            
        //Check 
        else if(nodeptr_out->value == LOGIC_1 && Xnum == 0) { 
            if(dNum % 2 == 0 && dbNum % 2 == 0) {
                if(oneNum % 2 == 1) {
                    if(nodeptr_out->value == LOGIC_0) {
                        return false;
                    }
                }
            } 
            else if(dNum == dbNum && oneNum % 2 == 0) { 
                if(nodeptr_out->value == LOGIC_0) {
                    return false;
                }
            }
        } 
        else if(nodeptr_out->value == LOGIC_0 && Xnum == 0) { 
            if(dNum % 2 == 0 && dbNum % 2 == 0) {
                if(oneNum % 2 == 0) {
                    if(nodeptr_out->value == LOGIC_1) {
                        return false;
                    }
                }
                } 
                else if(dNum == dbNum && oneNum % 2 == 1) {
                    if(nodeptr_out->value == LOGIC_1) {
                        return false;
                    }
                }
            }
        }

        //OR 
            
        else if(nodeptr_out->type == GATE_OR){
            
            //Imply 

            if(nodeptr_out->value  == LOGIC_X ) { 
                if(oneNum>0 ||  (dNum>0&& dbNum>0)) {
                    nodeptr_out->value=LOGIC_1; 
                    imply.push_back(nodeptr_out->indx);
                } 
                else if (Xnum == 0 && zeroNum == nodeptr_out->fin) {
                    nodeptr_out->value=LOGIC_0; 
                    imply.push_back(nodeptr_out->indx);
                } 
                else if( (zeroNum+dNum) == nodeptr_out->fin) {
                    nodeptr_out->value=LOGIC_D; 
                    imply.push_back(nodeptr_out->indx);
                } 
                else if( (zeroNum+dbNum) == nodeptr_out->fin) {
                    nodeptr_out->value=LOGIC_DBAR; 
                    imply.push_back(nodeptr_out->indx);
                }
            }
            
            //Check 
            else if(nodeptr_out->value == LOGIC_1) {
                if(!(oneNum>0 ||  (dNum>0&& dbNum>0))) {
                  return false;
                }
            } 
            else if (nodeptr_out->value == LOGIC_0) {
                if(!(Xnum == 0 && zeroNum == nodeptr_out->fin)) {
                  return false;
                }
            }
        }

        //NOR
      
         else if(nodeptr_out->type == GATE_NOR) {
            
            //Imply 

            if(nodeptr_out->value == LOGIC_X ) { 
                if(oneNum>0 ||  (dNum>0&& dbNum>0)) {
                    nodeptr_out->value=LOGIC_0; 
                    imply.push_back(nodeptr_out->indx);
                } else if (Xnum == 0 && zeroNum == nodeptr_out->fin) {
                    nodeptr_out->value=LOGIC_1; 
                    imply.push_back(nodeptr_out->indx);
                } else if ((zeroNum+dNum) == nodeptr_out->fin) {
                    nodeptr_out->value=LOGIC_DBAR; 
                    imply.push_back(nodeptr_out->indx);
                } else if ((zeroNum+dbNum) == nodeptr_out->fin) {
                    nodeptr_out->value=LOGIC_D; 
                    imply.push_back(nodeptr_out->indx);
                }
            }

            // Check 

            else if(nodeptr_out->value == LOGIC_0) {
                if (!(oneNum>0 ||  (dNum>0&& dbNum>0))) {
                    return false;
                }
            } else if (nodeptr_out->value==LOGIC_1) {
                if (!(Xnum==0 && zeroNum==nodeptr_out->fin)) {
                    return false;
                }
            }
         }

         //NAND

         else if(nodeptr_out->type==GATE_NAND){
            // Imply 
            if(nodeptr_out->value ==LOGIC_X ) {
                if(zeroNum>0 ||  (dNum>0&& dbNum>0)) {
                    nodeptr_out->value=LOGIC_1; 
                    imply.push_back(nodeptr_out->indx);
                } else if(Xnum==0 && oneNum==nodeptr_out->fin) {
                    nodeptr_out->value=LOGIC_0; 
                    imply.push_back(nodeptr_out->indx);
                } else if((oneNum+dNum) == nodeptr_out->fin) {
                    nodeptr_out->value=LOGIC_DBAR; 
                    imply.push_back(nodeptr_out->indx);
                } else if((oneNum+dbNum) == nodeptr_out->fin) {
                    nodeptr_out->value=LOGIC_D; 
                    imply.push_back(nodeptr_out->indx);
                }
            }
            //Check 
            else if (nodeptr_out->value==LOGIC_1) { 
                if(!(zeroNum>0 ||  (dNum>0&& dbNum>0))) return false;
                } 
                else if (nodeptr_out->value==LOGIC_0) {
                if(!(Xnum==0 && oneNum==nodeptr_out->fin)) return false;
            }
        }

        //AND
      
        else if(nodeptr_out->type==GATE_AND){
            //Imply 
            if(nodeptr_out->value ==LOGIC_X ) { 
                if(zeroNum>0 ||  (dNum>0&& dbNum>0))    {nodeptr_out->value=LOGIC_0; imply.push_back(nodeptr_out->indx);}
                else if(Xnum==0 && oneNum==nodeptr_out->fin)   {nodeptr_out->value=LOGIC_1; imply.push_back(nodeptr_out->indx);}
                else if( (oneNum+dNum) == nodeptr_out->fin)    {nodeptr_out->value=LOGIC_D; imply.push_back(nodeptr_out->indx);}
                else if( (oneNum+dbNum) == nodeptr_out->fin) {nodeptr_out->value=LOGIC_DBAR; imply.push_back(nodeptr_out->indx);}
            }
            //Check
            else if(nodeptr_out->value==LOGIC_0) { if(!(zeroNum>0 ||  (dNum>0&& dbNum>0))) return false;}
            else if(nodeptr_out->value==LOGIC_1) {if(!(Xnum==0 && oneNum==nodeptr_out->fin)) return false;}
        }
    }  								
}
   
   return true;

}

bool Dalg(int level) {

    int time_elapsed = (clock() - dalg_recursion_tStart)/(CLOCKS_PER_SEC);
    if (time_elapsed > 1) {
       return false;
    }
 
    for ( int k=0;k< Nnodes;k++) {
        if( Node[k].assign_level > level) {
          Node[k].value=LOGIC_X; 
          Node[k].assign_level=-1;
        }
    }
 
    if(imply_and_check()) {
        for(int k=0; k< imply.size(); k++) { 
            Node[imply[k]].assign_level=level;  
        }     
    } 
    else {
        for(int i=0; i<imply.size(); i++ ){  
            Node[imply[i]].value=LOGIC_X; 
            Node[imply[i]].assign_level=-1; 
        }
        imply.clear(); 
        return false;
    }
 
    imply.clear();
    get_DFrontier();
    bool DatOut=false;
    NSTRUC *nodeptr;
    for(int i = 0; i<Npo; i++) {  //Check D or D' at output
        if(Poutput[i]->value==LOGIC_D||Poutput[i]->value==LOGIC_DBAR) {
            DatOut=true;
        }
    }

    //Propagate D frontier to output
    if(!DatOut) {
        if(Dfront.size()==0) {
            unassign(level-1); 
            return false;
        } 
        else {
            int iter_while= 0;
            for(int i = 0; i<Dfront.size(); i++) {
          
            nodeptr = &Node[Dfront[i]];
          
            int dNum=0; 
            int dbNum=0, oneNum=0, zeroNum=0;
            for(int j=0;j<(nodeptr->fin); j++ ) {
                if(nodeptr->unodes[j]->value == LOGIC_D) dNum++;
                else if(nodeptr->unodes[j]->value == LOGIC_DBAR) dbNum++;
                else if(nodeptr->unodes[j]->value == LOGIC_1) oneNum++;
                else if(nodeptr->unodes[j]->value == LOGIC_0) zeroNum++;
            }
          
            if ( (dNum==0 && dbNum==0)|| nodeptr->value!=LOGIC_X ||(dNum>0 && dbNum>0) ) {
                continue;
            }
          
            if(dNum>0) {
                if(nodeptr->type == GATE_OR || nodeptr->type== GATE_AND || nodeptr->type==GATE_BRANCH || nodeptr->type==GATE_XOR || nodeptr->type==GATE_XNOR || nodeptr->type==GATE_BUF)       {   nodeptr->value=LOGIC_D; } 
                else if(nodeptr->type ==GATE_NOR || nodeptr->type==GATE_NAND || nodeptr->type==GATE_NOT) {   nodeptr->value=LOGIC_DBAR; }
             
                imply.push_back(nodeptr->indx);
                for(int j=0;j<(nodeptr->fin); j++ ){
                    if( nodeptr->unodes[j]->value == LOGIC_X) {
                        if(nodeptr->type == GATE_OR || nodeptr->type==GATE_NOR || nodeptr->type==GATE_XOR) { nodeptr->unodes[j]->value=LOGIC_0; } 
                        else if(nodeptr->type ==GATE_NAND || nodeptr->type==GATE_AND || nodeptr->type==GATE_XNOR ) { nodeptr->unodes[j]->value=LOGIC_1; }
                        imply.push_back(nodeptr->unodes[j]->indx);
                    }
                }	           
            }
          
            else if(dbNum>0) {
                if(nodeptr->type == GATE_OR || nodeptr->type==GATE_AND  || nodeptr->type==GATE_BRANCH || nodeptr->type==GATE_XOR || nodeptr->type==GATE_XNOR )      {   nodeptr->value=LOGIC_DBAR; } 
                else if(nodeptr->type ==GATE_NOR || nodeptr->type==GATE_NAND || nodeptr->type==GATE_NOT) {   nodeptr->value=LOGIC_D; }
             
                imply.push_back(nodeptr->indx);
                for(int j=0;j<(nodeptr->fin); j++ ){
                if( nodeptr->unodes[j]->value ==LOGIC_X){
                    if(nodeptr->type ==GATE_OR || nodeptr->type==GATE_NOR || nodeptr->type==GATE_XOR) { nodeptr->unodes[j]->value=LOGIC_0; } 
                    else if(nodeptr->type == GATE_NAND || nodeptr->type==GATE_AND || nodeptr->type==GATE_XNOR ) { nodeptr->unodes[j]->value=LOGIC_1; }
                    imply.push_back(nodeptr->unodes[j]->indx);
                }
            }
        }
        
        int pop_ind=Dfront[0];
          
        if(Dalg(level+1)) {
            return true;
        } 
        else {  
            unassign(level); 
            get_DFrontier();
        }
    }
    
    unassign(level-1);
    return false;
    }
}
    //check Jfront
    vector<int> temp2; temp2.clear();
    for(int i=0;i <Jfront.size();i++) {
        int Xnum=0;
        int dNum=0;
        int dbNum=0;
        int oneNum=0;
        int zeroNum=0;
        nodeptr = &Node[Jfront[i]];
        for(int j=0;j< nodeptr->fin ;j++){
            if      (nodeptr->unodes[j]->value==LOGIC_X) Xnum++;
            else if (nodeptr->unodes[j]->value==LOGIC_D) dNum++;
            else if (nodeptr->unodes[j]->value==LOGIC_DBAR) dbNum++;
            else if (nodeptr->unodes[j]->value==LOGIC_1) oneNum++; 
            else if (nodeptr->unodes[j]->value==LOGIC_0) zeroNum++;
        }
        if(Node[Jfront[i]].value==LOGIC_1 && nodeptr->type==GATE_OR) { //OR
            if(Xnum>0 && !(dNum>0&& dbNum>0) && oneNum==0) {temp2.push_back(Jfront[i]);}
        }
        else if(Node[Jfront[i]].value==LOGIC_1 && nodeptr->type==GATE_NAND) { //NAND
            if(Xnum>0 && !(dNum>0&& dbNum>0) && zeroNum==0) {temp2.push_back(Jfront[i]);}
        }
        else if(Node[Jfront[i]].value==LOGIC_0 && nodeptr->type==GATE_NOR){ //NOR
            if(Xnum>0 && !(dNum>0&& dbNum>0) && oneNum==0 ) {temp2.push_back(Jfront[i]);}
        }
        else if(Node[Jfront[i]].value==LOGIC_0 && nodeptr->type==GATE_AND){ //AND
            if(Xnum>0 && !(dNum>0&& dbNum>0) && zeroNum==0) {temp2.push_back(Jfront[i]);}
        }
    }
    Jfront= temp2;
    if(temp2.size()==0) return true;
    for(int i=0;i <temp2.size();i++){
        int Xnum=0;
        int dNum=0;
        int dbNum=0;
        int oneNum=0;
        int zeroNum=0;
        nodeptr = &Node[temp2[i]];
        for(int j=0;j< nodeptr->fin ;j++){
            if(nodeptr->unodes[j]->value==LOGIC_X){
                if(nodeptr->type==GATE_OR || nodeptr->type==GATE_NOR){
                    nodeptr->unodes[j]->value =  LOGIC_1;   imply.push_back(nodeptr->unodes[j]->indx);	
                    if(Dalg(level+1)) {return true;} else { unassign(level);}
                    nodeptr->unodes[j]->value =  LOGIC_0;   imply.push_back(nodeptr->unodes[j]->indx);
                    nodeptr->unodes[j]->assign_level= level;
                }
                else if(nodeptr->type==GATE_AND || nodeptr->type==GATE_NAND){
                    nodeptr->unodes[j]->value = LOGIC_0;   imply.push_back(nodeptr->unodes[j]->indx);	
                    if(Dalg(level+1)) {return true;} else { unassign(level);}
                    nodeptr->unodes[j]->value =  LOGIC_1;   imply.push_back(nodeptr->unodes[j]->indx);
                    nodeptr->unodes[j]->assign_level= level;
                }
            }
        }
        imply.clear();
        unassign(level-1);
        return false;
    }
}

 
bool DalgCall(pair<int,int> fault){

    Dfront.clear();
    Jfront.clear(); 
    imply.clear();
    dalg_recursion_tStart = clock();
    Dalg_count=0;
    for(int k=0;k< Nnodes; k++){
        Node[k].value= LOGIC_X; 
        Node[k].assign_level = -1;
    }
    NSTRUC *nodeptr =&Node[(int) (fault.first)];
    check_fault = fault;
    for(int k=0;k< nodeptr->fout;k++){
        Dfront.push_back(nodeptr->dnodes[k]->indx);      // Set D Frontier
    }
    if (fault.second == 0) {
        nodeptr->value= LOGIC_D; 
        if(nodeptr->type==GATE_BRANCH) {
            nodeptr->unodes[0]->value=LOGIC_1; 
        }
        imply.push_back(nodeptr->indx);
    } else {  
        nodeptr->value= LOGIC_DBAR; 
        if(nodeptr->type==GATE_BRANCH) {
            nodeptr->unodes[0]->value=LOGIC_0; 
        }
        imply.push_back(nodeptr->indx);
    }

    bool find=Dalg(0);

    if(find) { 
        vector<int> temp; 
        temp.clear();
        for(int i=0;i<Npi  ;i++){
            if(Pinput[i]->value==LOGIC_D) {
            temp.push_back(1);
            } else if(Pinput[i]->value==LOGIC_DBAR) {
            temp.push_back(0);
            } else {
            temp.push_back(Pinput[i]->value);
            }
        }
        test_vectors.push_back(temp);
        return true;
    }
    else return false;
    }

int five_value_forward_imply(std::list<int> &dalgo_D_Frontiers, std::list<int> &dalgo_J_Frontiers, int node_index){
    NSTRUC *nodePointer;
    for(int i = 1; i <= Nlevels; i++){ 
        for(int j = 0; j < Nnodes; j++){ 
            nodePointer = &Node[j];
            if(nodePointer->level == i){
                int value = 0;
                int nxt_inp_value = 0;
                switch(nodePointer->type){
                    case XOR:
                        value = nodePointer->unodes[0]->eval_value;
                        for(int i = 1; i < nodePointer->fin; i++){
                            nxt_inp_value = nodePointer->unodes[i]->eval_value;
                            if(value == 0) value = nxt_inp_value;
                            else if(value == 1){
                                if(nxt_inp_value == 2) value = 2;
                                else if(nxt_inp_value == 0 | nxt_inp_value == 1) value = !(nxt_inp_value);
                                else value = (nxt_inp_value == 3)?4:3; 
                            }
                            else if(value == 2) value = 2;
                            else if(value == 3){
                                if(nxt_inp_value == 2) value = 2;
                                else if(nxt_inp_value == 0 | nxt_inp_value == 1) value = (nxt_inp_value == 0)?3:4;
                                else value = (nxt_inp_value == 3)?0:1;
                            }
                            else if(value == 4){
                                if(nxt_inp_value == 2) value = 2;
                                else if(nxt_inp_value == 0 | nxt_inp_value == 1) value = (nxt_inp_value == 0)?4:3;
                                else value = (nxt_inp_value == 3)?1:0;
                            }
                        }
                        break;
                    case OR:
                        value = nodePointer->unodes[0]->eval_value; 
                        for(int i = 1; i < nodePointer->fin; i++){
                            nxt_inp_value = nodePointer->unodes[i]->eval_value;
                            if(value == 0) value = nxt_inp_value;
                            else if(value == 1) value = 1;
                            else if(value == 2){
                                if(nxt_inp_value == 1) value = 1;
                                else value = 2;
                            }
                            else if(value == 3){
                                if(nxt_inp_value == 1) value = 1;
                                else if(nxt_inp_value == 0) value = 3;
                                else if(nxt_inp_value == 2) value = 2;
                                else if(nxt_inp_value == 3) value = 3;
                                else if(nxt_inp_value == 4) value = 1;
                            }
                            else if(value == 4){
                                if(nxt_inp_value == 1) value = 1;
                                else if(nxt_inp_value == 0) value = 4;
                                else if(nxt_inp_value == 2) value = 2;
                                else if(nxt_inp_value == 3) value = 1;
                                else if(nxt_inp_value == 4) value = 4;
                            }
                        }
                        break;
                    case NOR:
                        value = nodePointer->unodes[0]->eval_value; 
                        for(int i = 1; i < nodePointer->fin; i++){
                            nxt_inp_value = nodePointer->unodes[i]->eval_value;
                            if(value == 0) value = nxt_inp_value;
                            else if(value == 1) value = 1;
                            else if(value == 2){
                                if(nxt_inp_value == 1) value = 1;
                                else value = 2;
                            }
                            else if(value == 3){
                                if(nxt_inp_value == 1) value = 1;
                                else if(nxt_inp_value == 0) value = 3;
                                else if(nxt_inp_value == 2) value = 2;
                                else if(nxt_inp_value == 3) value = 3;
                                else if(nxt_inp_value == 4) value = 1;
                            }
                            else if(value == 4){
                                if(nxt_inp_value == 1) value = 1;
                                else if(nxt_inp_value == 0) value = 4;
                                else if(nxt_inp_value == 2) value = 2;
                                else if(nxt_inp_value == 3) value = 1;
                                else if(nxt_inp_value == 4) value = 4;
                            }
                        }
                        if(value ==2) value = 2;
                        else if(value == 3 | value == 4) value = (value == 3)? 4:3;
                        else value = !value;
                        break;
                    case NOT:
                        nxt_inp_value = nodePointer->unodes[0]->eval_value;
                        if(nxt_inp_value == 2) value = 2;
                        else if(nxt_inp_value == 0 | nxt_inp_value == 1) value = !(nxt_inp_value);
                        else value = (nxt_inp_value == 3)?4:3;
                        break;
                    case NAND:
                        value = nodePointer->unodes[0]->eval_value; 
                        for(int i = 1; i < nodePointer->fin; i++){
                            nxt_inp_value = nodePointer->unodes[i]->eval_value;
                            if(value == 0) value = 0;
                            else if(value == 1) value = nxt_inp_value;
                            else if(value == 2){
                                if(nxt_inp_value == 0) value = 0;
                                else value = 2;
                            }
                            else if(value == 3){
                                if(nxt_inp_value == 1) value = 3;
                                else if(nxt_inp_value == 0) value = 0;
                                else if(nxt_inp_value == 2) value = 2;
                                else if(nxt_inp_value == 3) value = 3;
                                else if(nxt_inp_value == 4) value = 0;
                            }
                            else if(value == 4){
                                if(nxt_inp_value == 1) value = 4;
                                else if(nxt_inp_value == 0) value = 0;
                                else if(nxt_inp_value == 2) value = 2;
                                else if(nxt_inp_value == 3) value = 0;
                                else if(nxt_inp_value == 4) value = 4;
                            }
                        }
                        if(value ==2) value = 2;
                        else if(value == 3 | value == 4) value = (value == 3)? 4:3;
                        else value = !value;
                        break;
                    case AND:
                        value = nodePointer->unodes[0]->eval_value; 
                        for(int i = 1; i < nodePointer->fin; i++){
                            nxt_inp_value = nodePointer->unodes[i]->eval_value;
                            if(value == 0) value = 0;
                            else if(value == 1) value = nxt_inp_value;
                            else if(value == 2){
                                if(nxt_inp_value == 0) value = 0;
                                else value = 2;
                            }
                            else if(value == 3){
                                if(nxt_inp_value == 1) value = 3;
                                else if(nxt_inp_value == 0) value = 0;
                                else if(nxt_inp_value == 2) value = 2;
                                else if(nxt_inp_value == 3) value = 3;
                                else if(nxt_inp_value == 4) value = 0;
                            }
                            else if(value == 4){
                                if(nxt_inp_value == 1) value = 4;
                                else if(nxt_inp_value == 0) value = 0;
                                else if(nxt_inp_value == 2) value = 2;
                                else if(nxt_inp_value == 3) value = 0;
                                else if(nxt_inp_value == 4) value = 4;
                            }
                        }
                        break;
                    case XNOR:
                        value = nodePointer->unodes[0]->eval_value;
                        for(int i = 1; i < nodePointer->fin; i++){
                            nxt_inp_value = nodePointer->unodes[i]->eval_value;
                            if(value == 0) value = nxt_inp_value;
                            else if(value == 1){
                                if(nxt_inp_value == 2) value = 2;
                                else if(nxt_inp_value == 0 | nxt_inp_value == 1) value = !(nxt_inp_value);
                                else value = (nxt_inp_value == 3)?4:3; 
                            }
                            else if(value == 2) value = 2;
                            else if(value == 3){
                                if(nxt_inp_value == 2) value = 2;
                                else if(nxt_inp_value == 0 | nxt_inp_value == 1) value = (nxt_inp_value == 0)?3:4;
                                else value = (nxt_inp_value == 3)?0:1;
                            }
                            else if(value == 4){
                                if(nxt_inp_value == 2) value = 2;
                                else if(nxt_inp_value == 0 | nxt_inp_value == 1) value = (nxt_inp_value == 0)?4:3;
                                else value = (nxt_inp_value == 3)?1:0;
                            }
                        }
                        if(value ==2) value = 2;
                        else if(value == 3 | value == 4) value = (value == 3)? 4:3;
                        else value = !value;
                        break;
                    case BUFFER:
                        value = nodePointer->unodes[0]->eval_value;
                        break;
                    case BRCH:
                        value = nodePointer->unodes[0]->eval_value;
                        break;
                }

                if(nodePointer->eval_value == 2){
                    events[nodePointer->indx] = nodePointer->eval_value;
                    nodePointer->eval_value = value;
                }
                else{
                    if(value != 2) {
                        if(nodePointer->indx == node_index){
                            if(nodePointer->eval_value == 0 | nodePointer->eval_value == 4){
                                if(value == 1 | value == 3){
                                    return 0;
                                }
                            }
                            else if(value == 0 | value == 4){
                                return 0;
                            }
                        }
                        else if(nodePointer->eval_value != value) return 0;
                    }
                }

                // cout << nodePointer->num << ": " << nodePointer->eval_value << endl;

                int D_count = 0;
                int Dbar_count = 0;
                if(nodePointer->eval_value == 2){ // checking if the node is D frontier
                    for(int k = 0; k < nodePointer->fin; k++){
                        if(nodePointer->unodes[k]->eval_value == 3) D_count++;
                        if(nodePointer->unodes[k]->eval_value == 4) Dbar_count++;
                    }
                    if(nodePointer->type == XOR | nodePointer->type == XNOR){
                        if((D_count + Dbar_count)%2 == 1){
                            dalgo_D_Frontiers.push_back(nodePointer->indx);
                        }
                    }
                    else{
                        if(D_count > 0){
                            if(Dbar_count == 0){
                                dalgo_D_Frontiers.push_back(nodePointer->indx);
                            }
                        }
                        else if(Dbar_count > 0){
                                dalgo_D_Frontiers.push_back(nodePointer->indx);
                        }
                    }
                }
                else { // checking if the node is J frontier
                    for(int k = 0; k < nodePointer->fin; k++){
                        if(nodePointer->unodes[k]->eval_value == 2){
                            dalgo_J_Frontiers.push_back(nodePointer->indx);
                            break;
                        }
                    }
                }
            }
        }
    }
}

int five_value_backward_imply(int node_index){
    NSTRUC *nodePointer;
    nodePointer = &Node[node_index];

    int output_value = nodePointer->eval_value;

    int inp_x_count = 0;
    int inp_x_count_index;
    int xor_inp_eval;
    switch(nodePointer->type){
        case IPT:
            return 1;
            break;                      
        case BRCH:
            if(output_value == 3 | output_value == 4){
                for(int i = 0; i < nodePointer->fin; i++){
                    events[nodePointer->unodes[i]->indx] = nodePointer->unodes[i]->eval_value;
                    nodePointer->unodes[i]->eval_value = (output_value == 3)?1:0;
                    to_imply.push_back(nodePointer->unodes[i]->indx);
                }
            }
            else{
                for(int i = 0; i < nodePointer->fin; i++){
                    events[nodePointer->unodes[i]->indx] = nodePointer->unodes[i]->eval_value;
                    nodePointer->unodes[i]->eval_value = output_value;
                    to_imply.push_back(nodePointer->unodes[i]->indx);
                }
            }
            break;
        case XOR:
            for(int i = 0; i < nodePointer->fin; i++){
                if(nodePointer->unodes[i]->eval_value == 2){
                    inp_x_count++;
                    inp_x_count_index = i;
                }
            }
            if(inp_x_count == 1){
                if(output_value == 3 | output_value == 4) output_value = (output_value == 3)?1:0;
                xor_inp_eval = nodePointer->unodes[0]->eval_value;
                for(int i = 1; i < nodePointer->fin; i++){
                    if(nodePointer->unodes[i]->eval_value != 2){
                        xor_inp_eval = xor_inp_eval^nodePointer->unodes[i]->eval_value;
                    }
                }
                if(xor_inp_eval == 0){
                    events[nodePointer->unodes[inp_x_count_index]->indx] = nodePointer->unodes[inp_x_count_index]->eval_value;
                    nodePointer->unodes[inp_x_count_index]->eval_value = (output_value == 1)?1:0;
                    to_imply.push_back(nodePointer->unodes[inp_x_count_index]->indx); 
                }
                else {
                    events[nodePointer->unodes[inp_x_count_index]->indx] = nodePointer->unodes[inp_x_count_index]->eval_value;
                    nodePointer->unodes[inp_x_count_index]->eval_value = (output_value == 1)?0:1;
                    to_imply.push_back(nodePointer->unodes[inp_x_count_index]->indx); 
                }
            }
            else return 1;
            break;
        case OR:
            if(output_value == 3 | output_value == 4) output_value = (output_value == 3)?1:0;
            if(output_value == 0){
                for(int i = 0; i < nodePointer->fin; i++){
                    if(nodePointer->unodes[i]->eval_value == 2){
                        events[nodePointer->unodes[i]->indx] = nodePointer->unodes[i]->eval_value;
                        nodePointer->unodes[i]->eval_value = 0;
                        to_imply.push_back(nodePointer->unodes[i]->indx);
                    }
                    else if(nodePointer->unodes[i]->eval_value != 0){
                        if(nodePointer->unodes[i]->ntype != 1) return 0;
                    }
                }
            }
            else{ // if the output_value is equal to one
                for(int i = 0; i < nodePointer->fin; i++){
                    if(nodePointer->unodes[i]->eval_value == 2){
                        inp_x_count++;
                        inp_x_count_index = i;
                    }
                }
                if(inp_x_count == 1){
                    events[nodePointer->unodes[inp_x_count_index]->indx] = nodePointer->unodes[inp_x_count_index]->eval_value;
                    nodePointer->unodes[inp_x_count_index]->eval_value = 1;
                    to_imply.push_back(nodePointer->unodes[inp_x_count_index]->indx);
                    
                }
                else return 1;
            }            
            break;
        case NOR:
            if(output_value == 3 | output_value == 4) output_value = (output_value == 3)?1:0;
            if(output_value == 1){
                for(int i = 0; i < nodePointer->fin; i++){
                    if(nodePointer->unodes[i]->eval_value == 2){
                        events[nodePointer->unodes[i]->indx] = nodePointer->unodes[i]->eval_value;
                        nodePointer->unodes[i]->eval_value = 0;
                        to_imply.push_back(nodePointer->unodes[i]->indx);
                       
                    }
                    else if(nodePointer->unodes[i]->eval_value != 0) return 0;
                }
            }
            else{ // if the output_value is equal to zero
                for(int i = 0; i < nodePointer->fin; i++){
                    if(nodePointer->unodes[i]->eval_value == 2){
                        inp_x_count++;
                        inp_x_count_index = i;
                    }
                }
                if(inp_x_count == 1){
                    events[nodePointer->unodes[inp_x_count_index]->indx] = nodePointer->unodes[inp_x_count_index]->eval_value;
                    nodePointer->unodes[inp_x_count_index]->eval_value = 1;
                    to_imply.push_back(nodePointer->unodes[inp_x_count_index]->indx);
                    
                }
            }
            break;
        case NOT:
            if(output_value == 3 | output_value == 4){
                for(int i = 0; i < nodePointer->fin; i++){
                    events[nodePointer->unodes[i]->indx] = nodePointer->unodes[i]->eval_value;
                    nodePointer->unodes[i]->eval_value = (output_value == 3)?0:1;
                    to_imply.push_back(nodePointer->unodes[i]->indx);
                   
                }
            }
            else{
                for(int i = 0; i < nodePointer->fin; i++){
                    events[nodePointer->unodes[i]->indx] = nodePointer->unodes[i]->eval_value;
                    nodePointer->unodes[i]->eval_value = !output_value;
                    to_imply.push_back(nodePointer->unodes[i]->indx);
                   
                }
            }
            break;
        case NAND:
            if(output_value == 3 | output_value == 4) output_value = (output_value == 3)?1:0;
            if(output_value == 0){
                for(int i = 0; i < nodePointer->fin; i++){
                    if(nodePointer->unodes[i]->eval_value == 2){
                        events[nodePointer->unodes[i]->indx] = nodePointer->unodes[i]->eval_value;
                        nodePointer->unodes[i]->eval_value = 1;
                        to_imply.push_back(nodePointer->unodes[i]->indx);
                       
                    }
                    else if(nodePointer->unodes[i]->eval_value != 1) return 0;
                }
            }
            else{ // if the output_value is equal to one
                for(int i = 0; i < nodePointer->fin; i++){
                    if(nodePointer->unodes[i]->eval_value == 2){
                        inp_x_count++;
                        inp_x_count_index = i;
                    }
                }
                if(inp_x_count == 1){
                    events[nodePointer->unodes[inp_x_count_index]->indx] = nodePointer->unodes[inp_x_count_index]->eval_value;
                    nodePointer->unodes[inp_x_count_index]->eval_value = 0;
                    to_imply.push_back(nodePointer->unodes[inp_x_count_index]->indx);
                    
                } 
            }
            break;
        case AND:
            if(output_value == 3 | output_value == 4) output_value = (output_value == 3)?1:0;
            if(output_value == 1){
                for(int i = 0; i < nodePointer->fin; i++){
                    if(nodePointer->unodes[i]->eval_value == 2){
                        events[nodePointer->unodes[i]->indx] = nodePointer->unodes[i]->eval_value;
                        nodePointer->unodes[i]->eval_value = 1;
                        to_imply.push_back(nodePointer->unodes[i]->indx);
                       
                    }
                    else if(nodePointer->unodes[i]->eval_value != 1) return 0;
                }
            }
            else{ // if the output_value is equal to zero
                for(int i = 0; i < nodePointer->fin; i++){
                    if(nodePointer->unodes[i]->eval_value == 2){
                        inp_x_count++;
                        inp_x_count_index = i;
                    }
                }
                if(inp_x_count == 1){
                    events[nodePointer->unodes[inp_x_count_index]->indx] = nodePointer->unodes[inp_x_count_index]->eval_value;
                    nodePointer->unodes[inp_x_count_index]->eval_value = 0;
                    to_imply.push_back(nodePointer->unodes[inp_x_count_index]->indx);
                    
                } 
            }            
            break;
        case XNOR:
            for(int i = 0; i < nodePointer->fin; i++){
                if(nodePointer->unodes[i]->eval_value == 2){
                    inp_x_count++;
                    inp_x_count_index = i;
                }
            }
            if(inp_x_count == 1){
                if(output_value == 3 | output_value == 4) output_value = (output_value == 3)?1:0;
                xor_inp_eval = nodePointer->unodes[0]->eval_value;
                for(int i = 1; i < nodePointer->fin; i++){
                    if(nodePointer->unodes[i]->eval_value != 2){
                        xor_inp_eval = xor_inp_eval^nodePointer->unodes[i]->eval_value;
                    }
                }
                if(xor_inp_eval == 0){
                    events[nodePointer->unodes[inp_x_count_index]->indx] = nodePointer->unodes[inp_x_count_index]->eval_value;
                    nodePointer->unodes[inp_x_count_index]->eval_value = (output_value == 1)?0:1;
                    to_imply.push_back(nodePointer->unodes[inp_x_count_index]->indx); 
                    
                }
                else {
                    events[nodePointer->unodes[inp_x_count_index]->indx] = nodePointer->unodes[inp_x_count_index]->eval_value;
                    nodePointer->unodes[inp_x_count_index]->eval_value = (output_value == 1)?1:0;
                    to_imply.push_back(nodePointer->unodes[inp_x_count_index]->indx); 
                    
                }
            }
            else return 1;
            break;
        case BUFFER:
            if(output_value == 3 | output_value == 4){
                for(int i = 0; i < nodePointer->fin; i++){
                    events[nodePointer->unodes[i]->indx] = nodePointer->unodes[i]->eval_value;
                    nodePointer->unodes[i]->eval_value = (output_value == 3)?1:0;
                   
                }
            }
            else{
                for(int i = 0; i < nodePointer->fin; i++){
                    events[nodePointer->unodes[i]->indx] = nodePointer->unodes[i]->eval_value;
                    nodePointer->unodes[i]->eval_value = output_value;
                   
                }
            }
            break;
    }
}

int dalgo_imply_and_check(std::list<int> &to_imply, std::list<int> &dalgo_D_Frontiers, std::list<int> &dalgo_J_Frontiers, int node_index){
    dalgo_D_Frontiers.clear();
    dalgo_J_Frontiers.clear();
    NSTRUC *nodePointer;
    list<int>::iterator it = to_imply.begin();

    for(int node_to_imply: to_imply){
        nodePointer = &Node[node_to_imply];
        if(five_value_backward_imply(nodePointer->indx) == 0){
            return 0;
        }
    }

    if(five_value_forward_imply(dalgo_D_Frontiers, dalgo_J_Frontiers, node_index) == 0){
        return 0;
    }

    to_imply.clear();
    return 1;
}

int dalgo(int node_index, int fault_val, map<int, int> backtrack_list, time_t ATPG_start_test){

    if(double(clock() - ATPG_start_test)/double(CLOCKS_PER_SEC) > 2){

        return 0;
    }

    dalgo_count++;

    list<int> dalgo_D_Frontiers, dalgo_J_Frontiers;
    map<int, int> backtrack_list_to_nxt_algo;
    map<int, int> backtrack_list_by_imply;
    int imply_and_check_Status = dalgo_imply_and_check(to_imply, dalgo_D_Frontiers, dalgo_J_Frontiers, node_index);
    backtrack_list_by_imply = events;
    if(imply_and_check_Status == 0){
        for(const auto& pair: events) Node[pair.first].eval_value = pair.second;
        for(const auto& pair: backtrack_list) Node[pair.first].eval_value = pair.second;
        backtrack_list.clear();
        events.clear();
        dalgo_count--;
        return 0;
    }
    else events.clear();

    int fault_at_po = 0;
    if(Node[node_index].ntype == 3) fault_at_po = 1;
    for(int i = 0; i < Npo; i++){
        if(Poutput[i]->eval_value == 3 | Poutput[i]->eval_value == 4) fault_at_po = 1;
    }
    
    if(fault_at_po == 0){
        if(dalgo_D_Frontiers.empty()){
            for(const auto& pair: backtrack_list) Node[pair.first].eval_value = pair.second;
            for(const auto& pair: backtrack_list_by_imply) Node[pair.first].eval_value = pair.second;
            backtrack_list.clear();
            backtrack_list_by_imply.clear();
            dalgo_count--;
            return 0;
        }
        for(int d_fron: dalgo_D_Frontiers){
            for(int i = 0; i < Node[d_fron].fin; i++){
                if(Node[d_fron].unodes[i]->eval_value == 2){
                    backtrack_list_to_nxt_algo[Node[d_fron].unodes[i]->indx] = 2;
                    Node[d_fron].unodes[i]->eval_value = !controlling_value(Node[d_fron].unodes[i]->indx);
                    to_imply.push_back(Node[d_fron].unodes[i]->indx);
                }
            }
            if(dalgo(node_index, fault_val, backtrack_list_to_nxt_algo, ATPG_start_test) == 1){
                dalgo_count--;
                return 1;
            }
            if(double(clock() - ATPG_start_test)/double(CLOCKS_PER_SEC) > 2){
        
                return 0;
            }
            to_imply.clear();
        }
        dalgo_count --;
        for(const auto& pair: backtrack_list) Node[pair.first].eval_value = pair.second;
        for(const auto& pair: backtrack_list_by_imply) Node[pair.first].eval_value = pair.second;
        backtrack_list.clear();
        backtrack_list_by_imply.clear();
        return 0;
    }

    if(dalgo_J_Frontiers.empty()){
        dalgo_count--;
        return 1;
    }
   
    int j_fron_x_inp_count = 0;
    int j_fron = dalgo_J_Frontiers.front();
    for(int i = 0; i < Node[j_fron].fin; i++){
        if(Node[j_fron].unodes[i]->eval_value == 2){
            backtrack_list_to_nxt_algo[Node[j_fron].unodes[i]->indx] = 2;
            Node[j_fron].unodes[i]->eval_value = controlling_value(Node[j_fron].unodes[i]->indx);
            to_imply.push_back(Node[j_fron].unodes[i]->indx);
            if(dalgo(node_index, fault_val, backtrack_list_to_nxt_algo, ATPG_start_test) == 1){
                dalgo_count--;
                return 1;
            }
            if(double(clock() - ATPG_start_test)/double(CLOCKS_PER_SEC) > 2){
        
                return 0;
            }
            Node[j_fron].unodes[i]->eval_value = !controlling_value(Node[j_fron].unodes[i]->indx);
        
            to_imply.push_back(Node[j_fron].unodes[i]->indx);
        }
    }   
    if(dalgo(node_index, fault_val, backtrack_list_to_nxt_algo, ATPG_start_test) == 1){
        dalgo_count--;
        return 1;
    }
    if(double(clock() - ATPG_start_test)/double(CLOCKS_PER_SEC) > 2){

        return 0;
    }
    
    dalgo_count--;
    for(const auto& pair: backtrack_list) Node[pair.first].eval_value = pair.second;
    for(const auto& pair: backtrack_list_by_imply) Node[pair.first].eval_value = pair.second;
    backtrack_list.clear();
    backtrack_list_by_imply.clear();
    return 0;
}

// void dalg(){
//     time_t dalg_time;
//     cp[strlen(cp) - 1] = '\0';
//     int node_num;
//     int fault_val;
//     char *tp_outfile;

//     node_num = stoi(strtok(cp, " "));
//     fault_val = stoi(strtok(NULL, " "));
//     tp_outfile = strtok(NULL, " ");

//     // lev();  
//     char* original_cp = cp;
//     cp = (char*)"temp_level.out";  // dummy file that will get overwritten each time
//     lev();
//     cp = original_cp;

//     ofstream write_tp_outfile(tp_outfile);
    
//     int node_index;
//     NSTRUC *nodePointer;
//     for(int i = 0; i < Nnodes; i++){ // All the lines set to 'x'
//         Node[i].eval_value = 2;
//         if(Node[i].num == node_num){
//             node_index = Node[i].indx;
//             Node[i].eval_value = (fault_val == 0)?3:4;
//         }
//     }

//     to_imply.push_back(node_index);

//     int main_dalg_status;
//     map<int, int> backtrack_list;
//     main_dalg_status = dalgo(node_index, fault_val, backtrack_list, dalg_time);
//     cout << "Final status: " << main_dalg_status << endl;




//     cout << "About to write test pattern..." << endl;
//     for (int i = 0; i < Npi; i++) {
//         cout << "PI " << Pinput[i]->num << ": ";
//         if (Pinput[i]->eval_value == 2) cout << "x" << endl;
//         else if (Pinput[i]->eval_value == 3) cout << "1" << endl;
//         else if (Pinput[i]->eval_value == 4) cout << "0" << endl;
//         else cout << Pinput[i]->eval_value << endl;
//     }
    




//     for(int i = 0; i < Npi; i++){
//         if(i < Npi -1) write_tp_outfile << Pinput[i]->num << ",";
//         else write_tp_outfile << Pinput[i]->num << endl;    
//     }
//     if(main_dalg_status == 1){
//         for(int i = 0; i < Npi; i++){
//             if(i < Npi -1){
//                 if(Pinput[i]->eval_value == 2) write_tp_outfile << "x" << ",";
//                 else if(Pinput[i]->eval_value == 3) write_tp_outfile << "1" << ",";
//                 else if(Pinput[i]->eval_value == 4) write_tp_outfile << "0" << ",";
//                 else write_tp_outfile << Pinput[i]->eval_value << ",";
//             }
//             else {
//                 if(Pinput[i]->eval_value == 2) write_tp_outfile << "x" << endl;
//                 else if(Pinput[i]->eval_value == 3) write_tp_outfile << "1" << endl;
//                 else if(Pinput[i]->eval_value == 4) write_tp_outfile << "0" << endl;
//                 else write_tp_outfile << Pinput[i]->eval_value << endl;
//             }    
//         }
//     }
//     write_tp_outfile.close();
// }


void dalg(){
    to_imply.clear();
    dalgo_count = 0;

    char saved_cp[MAXLINE];
    strncpy(saved_cp, cp, MAXLINE);

    char dummy_cp[] = "temp_level.out";
    cp = dummy_cp;
    lev();
    cp = saved_cp;

    int node_num, fault_val;
    char output_file[MAXLINE];

    if (sscanf(cp, "%d %d %s", &node_num, &fault_val, output_file) != 3) {
        // fprintf(stderr, "Usage: DALG <node_num> <0|1> <output_file>\n");
        return;
    }

    FILE *fp = fopen(output_file, "w");
    if (!fp) {
        // fprintf(stderr, "Cannot open output file: %s\n", output_file);
        return;
    }

    // fprintf(fp, "\n");

    int node_index;
    for (int i = 0; i < Nnodes; ++i) {
        Node[i].eval_value = 2;  // X
        if (Node[i].num == node_num) {
            node_index = Node[i].indx;
            Node[i].eval_value = (fault_val == 0) ? 3 : 4;  // D / D'
        }
    }

    to_imply.push_back(node_index);
    time_t dalg_start_time = clock();
    map<int, int> backtrack_list;
    int result = dalgo(node_index, fault_val, backtrack_list, dalg_start_time);

    for (int i = 0; i < Npi; ++i) {
        fprintf(fp, "%d", Pinput[i]->num);
        if (i < Npi - 1) fprintf(fp, ",");
    }
    fprintf(fp, "\n");

    if (result == 1) {
        for (int i = 0; i < Npi; ++i) {
            int val = Pinput[i]->eval_value;
            if (val == 2) fprintf(fp, "x");
            else if (val == 3) fprintf(fp, "1");
            else if (val == 4) fprintf(fp, "0");
            else fprintf(fp, "%d", val);
            if (i < Npi - 1) fprintf(fp, ",");
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}


// READ ../auto-tests-phase3/ckts/c17.ckt
// DALG 1 0 ../auto-tests-phase3/outputs/dalg/c17_dalg_1_0.out