#include "graph.h"
#include "user.h"
#include <string.h>
/***************************************************************************************************************************
 Main Function(Serial and Parallel Fault Simulation)
****************************************************************************************************************************/
void main(int argc,char **argv)
{
FILE *fisc,*fvec,*ffau,*fres,*fbench,*fbenchout;             //file pointers used for .isc file, .vec file, .faults file and resultfile
int Max,Opt,Npi,Npo,Total,Tfs;              //maxnode id,option,tot no of PIs,tot no of Pos,Tot no of input patterns& faults in.vec in.faults

NODE graph[Mnod]; 
int a,b,c,d;                             //random variables
NODE graphB[Mnod]; 
NODE graphDup[Mnod]; 


//POJECT PART 1
char* fname[Mfnam];
strncpy(fname, argv[1], Mfnam - 1); 
Npo = 0;
char* fbenchName[Mfnam];
sprintf(fbenchName, "%s.bench", fname);
fbench = fopen(fbenchName, "r");
Max = readBench(fbench, graphB, &Npo);
fclose(fbench);

// printCircuit(graphB,Max); 

// int oldToNewNodes[Max + 1];
// int count = createDuplicateGraph(graphB, graphDup, Max, oldToNewNodes); //create a duplicate graph
// // // printCircuit(graphDup,count);

// char* fname[Mfnam];
// strncpy(fname, argv[2], Mfnam - 1);
// faultInjectionToDuplicate(graphDup, graph, count, Max,oldToNewNodes, fname);

// createFaultFile(count, fname); //create fault file
// processBenchFiles(fname);
// createTestPatterns(fname);

printf("Done project part 1\n");


//PROJECT PART 2
//read test patterns from the generated files
//get random patterns file

int groupsizes = 4;
int groupSize = 1;
 
// int randNum = atoi(argv[3]); // Convert the argument to an integer

for (groupSize = 1; groupSize <= groupsizes; groupSize++) {
    
    int patternList[Mpt][Mlin]; // Array to store the patterns
    char fpatName[Mfnam];           // Buffer to store the file name

    sprintf(fpatName, "%s/%s_rand%d.pattern",fname,fname, groupSize);

  
    // FILE *fpat = fopen(fpatName, "r"); 
    // int tPt = readPatternFile(fpat, patternList);  // Read the patterns from the file
    // fclose(fpat);


    // char* fresName[Mfnam]; // Buffer to store the file name
    // sprintf(fresName, "%s/%s_rand%d.res", fname, fname, groupSize);
    // fres = fopen(fresName, "w");
    // FaultsSimulator(graphB, Max, tPt, Npo, patternList, fres); // Simulate the faults
    // fclose(fres);
}

printf("Done project part 2\n");

//PROJECT PART 3
for(groupSize = 1; groupSize <= groupsizes; groupSize++){
    char fpatName[Mfnam]; // Buffer to store the file name
    sprintf(fpatName, "%s/%s_rand%d.pattern", fname, fname, groupSize);
  
    FILE *ftest = fopen(fpatName, "r");
    char frestlnName[Mfnam]; // Buffer to store the file name
    sprintf(frestlnName, "%s/%s_rand%d.restln", fname, fname, groupSize);
    FILE *frestln = fopen(frestlnName, "w");
    readTestSetFile(ftest, fname, groupSize, frestln);


    fclose(ftest);
    fclose(frestln);
}

printf("Done project part 3\n");

clearCircuit(graph,Mnod);                                    //clear memeory for all members of graph
clearCircuit(graphB,Mnod);                                    //clear memeory for all members of graphB

return ;
}//end of main
/****************************************************************************************************************************/

