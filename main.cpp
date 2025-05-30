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

// int main()
int seed = 658; // default seed

int main(int argc, char* argv[])
{
        if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    int com;
    char cline[512], wstr[MAXLINE];

    while(!Done) {
        printf("\nCommand>");
        fgets(cline, sizeof(cline), stdin);
        if(sscanf(cline, "%s", wstr) != 1) continue;
        cp = wstr;
        while(*cp){
            *cp= Upcase(*cp);
            cp++;
        }
        std::string full_command = cline + strlen(wstr) + 1;
        cp = strdup(full_command.c_str());
        com = READ;
        while(com < NUMFUNCS && strcmp(wstr, command[com].name)) com++;
        if(com < NUMFUNCS) {
            if(command[com].state <= Gstate) (*command[com].fptr)();
            else printf("Execution out of sequence!\n");
        }
        // else system(cline);
   }
   return 0;
}

/*
mkdir build
cd build
cmake ..
make
cd ..

./build/simulator

READ auto-tests-phase1/ckts/x3mult.ckt
LEV auto-tests-phase1/temp.txt
LOGICSIM auto-tests-phase1/tpi/x3mult_t4.txt auto-tests-phase1/outputs/x3mult_t4_output.txt
*/