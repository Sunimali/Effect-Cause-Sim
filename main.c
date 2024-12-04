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
                        //structure used to store the ckt information in .isc file 
//PATTERN vector[Mpt];                      //structure used to store the input vectors information in .vec file 

//FAULT stuck[Mft];                      //structure used to store the faults information in .faults file
int a,b,c,d;                             //random variables
NODE graphB[Mnod]; 
NODE graphDup[Mnod]; 

//Read the .isc file and store the information in graph structure
// fisc=fopen(argv[1],"r");                           //file pointer to open .isc file 
// Max=0; 
// Max=readIsc(fisc,graph);                 //read .isc file and return index of last node in graph formed
// fclose(fisc);                            //close file pointer for .isc file
// printCircuit(graph,Max);  
               //print all members of graph structure

//Read the .vec file and store the information in  vector structure


fbench=fopen(argv[1],"r"); 
Npo = 0;
Max = readBench(fbench, graphB, &Npo);
fclose(fbench);

// printCircuit(graphB,Max); 


// //create a duplicate graph
// int oldToNewNodes[Max + 1];
// int count = createDuplicateGraph(graphB, graphDup, Max, oldToNewNodes);
// // // printCircuit(graphDup,count);

char* fname[Mfnam];
strncpy(fname, argv[2], Mfnam - 1);
// faultInjectionToDuplicate(graphDup, graph, count, Max,oldToNewNodes, fname);

// // // // //create fault file
// createFaultFile(count, fname);
// processBenchFiles(fname);
// createTestPatterns(fname);

printf("End of Fault Injection\n");

//read test patterns from the generated files
//get random patterns file

 
int randNum = atoi(argv[3]); // Convert the argument to an integer
int patternList[Mpt][Mlin]; // Array to store the patterns
char fpatName[Mfnam];           // Buffer to store the file name

    // Construct the file name using sprintf

sprintf(fpatName, "%s/%s_rand%d.pattern",fname,fname, randNum);

    // Open the file
FILE *fpat = fopen(fpatName, "r");
    // Read the patterns from the file
int tPt = readPatternFile(fpat, patternList);
fclose(fpat);

printf("End of Reading Patterns\n");

char* fresName[Mfnam]; // Buffer to store the file name
sprintf(fresName, "%s/%s_rand%d.res", fname, fname, randNum);
fres = fopen(fresName, "w");
FaultsSimulator(graphB, Max, tPt, Npo, patternList, fres);
fclose(fres);



// FILE *ftest = fopen(fpatName, "r");
// char frestlnName[Mfnam]; // Buffer to store the file name
// sprintf(frestlnName, "%s/%s_rand%d.restln", fname, fname, randNum);
// FILE *frestln = fopen(frestlnName, "w");
// readTestSetFile(ftest, fname, randNum, frestln);


// char* faults[500][Mlin];

// char* pattern = "01102";

// int mfaults = 0;
// mfaults =  extractPrimaryOutputsFaultList(fname, randNum, pattern, faults);
// printf("mfaulsts: %d\n", mfaults);

// int quarter = mfaults / 4;

// pickRandomFaults(frestln, quarter, faults, mfaults, fname, randNum);

// fclose(ftest);
// fclose(frestln);

clearCircuit(graph,Mnod);                                      //clear memeory for all members of graph
//for(a=0;a<Total;a++){ bzero(vector[a].piv,Mpi); }                //clear memeory for all members of vector
printf("End of Simulation\n");
return ;
}//end of main
/****************************************************************************************************************************/

