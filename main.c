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
fisc=fopen(argv[1],"r");                           //file pointer to open .isc file 
Max=0; 
Max=ReadIsc(fisc,graph);                 //read .isc file and return index of last node in graph formed
fclose(fisc);                            //close file pointer for .isc file
PrintCircuit(graph,Max);  
               //print all members of graph structure

//Read the .vec file and store the information in  vector structure


fbench=fopen(argv[2],"r"); 
Max = readBench(fbench, graphB);
PrintCircuit(graphB,Max); 

// fbenchout=fopen(argv[3],"w"); 
// writeBenchmarkFile(Max, graphB, fbenchout);
// fclose(fbenchout);

//create a duplicate graph
int oldToNewNodes[Max + 1];
printf("call createDuplicateGraph\n");
int count = createDuplicateGraph(graphB, graphDup, Max, oldToNewNodes);
printf("count = %d\n", count);
PrintCircuit(graphDup,count);

char* fname[Mfnam];
strncpy(fname, argv[3], Mfnam - 1);
printf("call faultInjection\n");
printf("fname = %s\n", fname);
faultInjection(graphDup, graph, count, Max,oldToNewNodes, fname);

// //create fault file
createFaultFile(count, fname);
processBenchFiles(fname);
// FILE *ftest;
// //read the .test file and store the information in  vector structure
// printf("call readTestFile\n");
// ftest = fopen("c17/c17_6_NANDto_AND.test", "r");
// printf("open file\n");
// PatternData* patterns[MAX_PATTERNS];
// int numPatterns;
// int capacity;
// numPatterns = readTestFile(ftest, patterns);
// fclose(ftest);
//Opt=0; 
createTestPatterns(fname);
//Opt=atoi(argv[3]);                          //getting the option from terminal for xval
//fres=fopen(argv[2],"w");                           //file pointer to open .out file for printing results

//Perform Logic Simulation for each Input vector and print the Pos .val in output file   

//fclose(fres);                                                  //close file pointer for .out file

ClearCircuit(graph,Mnod);                                      //clear memeory for all members of graph
//for(a=0;a<Total;a++){ bzero(vector[a].piv,Mpi); }                //clear memeory for all members of vector
return;
}//end of main
/****************************************************************************************************************************/

