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
Max = readBench(fbench, graphB);
// printCircuit(graphB,Max); 


//create a duplicate graph
int oldToNewNodes[Max + 1];
int count = createDuplicateGraph(graphB, graphDup, Max, oldToNewNodes);
// printCircuit(graphDup,count);

char* fname[Mfnam];
strncpy(fname, argv[2], Mfnam - 1);
faultInjection(graphDup, graph, count, Max,oldToNewNodes, fname);

// //create fault file
createFaultFile(count, fname);
processBenchFiles(fname);
createTestPatterns(fname);


//Perform Logic Simulation for each Input vector and print the Pos .val in output file   

//fclose(fres);                                                  //close file pointer for .out file

clearCircuit(graph,Mnod);                                      //clear memeory for all members of graph
//for(a=0;a<Total;a++){ bzero(vector[a].piv,Mpi); }                //clear memeory for all members of vector
return;
}//end of main
/****************************************************************************************************************************/

