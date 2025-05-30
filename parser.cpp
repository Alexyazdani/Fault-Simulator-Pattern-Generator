/*
Alexander Yazdani
USC Spring 2025
EE658
*/

#include "parser.h"

/*-----------------------------------------------------------------------
input: nothing
output: nothing
called by: cread
description:
    This routine clears the memory space occupied by the previous circuit
    before reading in new one. It frees up the dynamic arrays Node.unodes,
    Node.dnodes, Node.flist, Node, Pinput, Poutput, and Tap.
-----------------------------------------------------------------------*/
void clear(){
    int i;
    for(i = 0; i<Nnodes; i++) {
        free(Node[i].unodes);
        free(Node[i].dnodes);
    }
    free(Node);
    free(Pinput);
    free(Poutput);
    Gstate = EXEC;
}

/*-----------------------------------------------------------------------
input: nothing
output: nothing
called by: cread
description:
    This routine allocatess the memory space required by the circuit
    description data structure. It allocates the dynamic arrays Node,
    Node.flist, Node, Pinput, Poutput, and Tap. It also set the default
    tap selection and the fanin and fanout to 0.
-----------------------------------------------------------------------*/
void allocate(){
    int i;
    Node = (NSTRUC *) malloc(Nnodes * sizeof(NSTRUC));
    Pinput = (NSTRUC **) malloc(Npi * sizeof(NSTRUC *));
    Poutput = (NSTRUC **) malloc(Npo * sizeof(NSTRUC *));
    for(i = 0; i<Nnodes; i++) {
        Node[i].indx = i;
        Node[i].fin = Node[i].fout = 0;
    }
}

/*-----------------------------------------------------------------------
input: circuit description file name
output: nothing
called by: main
description:
    This routine reads in the circuit description file and set up all the
    required data structure. It first checks if the file exists, then it
    sets up a mapping table, determines the number of nodes, PI's and PO's,
    allocates dynamic data arrays, and fills in the structural information
    of the circuit. In the ISCAS circuit description format, only upstream
    nodes are specified. Downstream nodes are implied. However, to facilitate
    forward implication, they are also built up in the data structure.
    To have the maximal flexibility, three passes through the circuit file
    are required: the first pass to determine the size of the mapping table
    , the second to fill in the mapping table, and the third to actually
    set up the circuit information. These procedures may be simplified in
    the future.
-----------------------------------------------------------------------*/
std::string inp_name = "";
void cread(){
    char buf[MAXLINE];
    int ntbl, *tbl, i, j, k, nd, tp, fo, fi, ni = 0, no = 0;
    FILE *fd;
    NSTRUC *np;
    cp[strlen(cp)-1] = '\0';
    if((fd = fopen(cp,"r")) == NULL){
        printf("File does not exist!\n");
        return;
    }
    inp_name = cp;
    
    if(Gstate >= CKTLD) clear();
    Nnodes = Npi = Npo = ntbl = 0;
    while(fgets(buf, MAXLINE, fd) != NULL) {
        if(sscanf(buf,"%d %d", &tp, &nd) == 2) {
            if(ntbl < nd) ntbl = nd;
            Nnodes ++;
            if(tp == PI) Npi++;
            else if(tp == PO) Npo++;
        }
    }
    tbl = (int *) malloc(++ntbl * sizeof(int));
    
    fseek(fd, 0L, 0);
    i = 0;
    while(fgets(buf, MAXLINE, fd) != NULL) {
        if(sscanf(buf,"%d %d", &tp, &nd) == 2) tbl[nd] = i++;
    }
    allocate();

    fseek(fd, 0L, 0);
    while(fscanf(fd, "%d %d", &tp, &nd) != EOF) {
        np = &Node[tbl[nd]];
        np->num = nd;
        
        if(tp == PI) Pinput[ni++] = np;
        else if(tp == PO) Poutput[no++] = np;
        
        switch(tp) {
            case PI:
            case PO:
            case GATE:
                fscanf(fd, "%d %d %d", &np->type, &np->fout, &np->fin);
                break;
            case FB:
                np->fout = np->fin = 1;
                fscanf(fd, "%d", &np->type);
                break;
            default:
                printf("Unknown node type!\n");
                exit(-1);
        }
        np->unodes = (NSTRUC **) malloc(np->fin * sizeof(NSTRUC *));
        np->dnodes = (NSTRUC **) malloc(np->fout * sizeof(NSTRUC *));
        for(i = 0; i < np->fin; i++) {
            fscanf(fd, "%d", &nd);
            np->unodes[i] = &Node[tbl[nd]];
        }
        for(i = 0; i < np->fout; np->dnodes[i++] = NULL);
    }
    for(i = 0; i < Nnodes; i++) {
        for(j = 0; j < Node[i].fin; j++) {
            np = Node[i].unodes[j];
            k = 0;
            while(np->dnodes[k] != NULL) k++;
            np->dnodes[k] = &Node[i];
        }
    }
    for (int i = 0; i < Nnodes; ++i) {
        Node[i].index = i;
    }
    fclose(fd);
    Gstate = CKTLD;
    free(tbl);
    printf("==> OK");
}