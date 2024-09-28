#include "graph.h"
#include "user.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
/***************************************************************************************************************************
 * write netlist into a BenchmarkFile
 * ****************************************************************************************************************************/

int writeBenchmarkFile(int max, NODE* graph, FILE* fbench)
{
    int i;
    fprintf(fbench, "#\n#\n#\n#\n\n");
    for (i = 1; i <= max; i++)  // write primary inputs
    {
        if (graph[i].Type == 1)
        {
            fprintf(fbench, "INPUT(%d)\n", i);
        }

    }
    fprintf(fbench, "\n");

    for (i = 1; i <= max; i++)
    {
        if (graph[i].Po == 1) // write primary outputs
        {
            fprintf(fbench, "OUTPUT(%d)\n", i);
        }

    }
    fprintf(fbench, "\n");

    for (i = 1; i <= max; i++) // write the rest of the nodes
    {
        if (graph[i].Type != 0 && graph[i].Type != 1) //skip primary inputs & unknown nodes
        {
            fprintf(fbench, "%d = ", i);
            printNameFromTypeId(graph[i].Type, fbench);
            fprintf(fbench, "(");
            printFanInList(graph[i].Fin, fbench);
            fprintf(fbench, ")\n");

        }
    }
    return 0;
} // end of writeBenchmarkFile

/***************************************************************************************************************************
 * find the type of the node
 * ****************************************************************************************************************************/
void printNameFromTypeId(int type, FILE* fbench)
{
    switch (type)
    {
    case 1:
        fprintf(fbench, "INPuT");
        break;
    case 2:
        fprintf(fbench, "AND");
        break;
    case 3:
        fprintf(fbench, "NAND");
        break;
    case 4:
        fprintf(fbench, "OR");
        break;
    case 5:
        fprintf(fbench, "NOR");
        break;
    case 6:
        fprintf(fbench, "XOR");
        break;
    case 7:
        fprintf(fbench, "XNOR");
        break;
    case 8:
        fprintf(fbench, "BUFF");
        break;
    case 9:
        fprintf(fbench, "NOT");
        break;
    case 10:
        fprintf(fbench, "FROM");
        break;
    default:
        fprintf(fbench, "UNKNOWN");
        break;
    }
    return;
} // end of printNameFromTypeId

/***************************************************************************************************************************
 * print fanin list
 * ****************************************************************************************************************************/

void printFanInList(LIST* Fin, FILE* fbench) {
    LIST* temp;
    for (temp = Fin; temp != NULL; temp = temp->next)
    {
        fprintf(fbench, "%d", temp->id);
        if (temp->next != NULL)
        {
            fprintf(fbench, ",");
        }
    }
}
// end of printFanInList

/***************************************************************************************************************************
 * create a duplicate graph
 * ****************************************************************************************************************************/

int createDuplicateGraph(NODE* graph, NODE* graphDup, int max, int oldToNewNodes[]) {

    int i;
    int count = 0;
     // Declare the array to store the mapping of old nodes to new nodes
    int poCount = 0;

    // Initialize all elements to 0
    for (i = 0; i <= max; i++) {
        oldToNewNodes[i] = 0;
    }
    // intialize all nodes in graph structure
    for (i = 0; i < Mnod; i++)
    {
        InitializeCircuit(graphDup, i);
    }

    int j = 1;
    for (i = 1; i <= max; i++)
    {
        if(graph[i].Po == 1){
            poCount = poCount + 1;
        }
        if (graph[i].Type == 1){ //copy primary inputs
            graphDup[j] = graph[i];
            oldToNewNodes[i] = j;
            count = count + 1;
            j++;

        } else if (graph[i].Type == 0){ //skip unknown nodes
            continue;
        } else { //copy the rest of the nodes
            graphDup[j] = graph[i];
            graphDup[j + 1] = graph[i];
            deepCopyNode(&graphDup[j], &graph[i]);
            deepCopyNode(&graphDup[j + 1], &graph[i]);

            oldToNewNodes[i] = j;

            count = count + 2;
            j = j + 2;

        }
    }
    //update fanin and fanout lists
    updateFanInFanOut(graph, graphDup, max, count, oldToNewNodes);

    //xor gates count
    int xorCount = poCount;
   
    // add or gate to combine the xor gates
    int final = count + xorCount + 1; //final node count
    addXorComponents(graphDup, count, xorCount, final);
    return final;
}
// end of createDuplicateGraph

/***************************************************************************************************************************
 * update fanin and fanout lists
 * ****************************************************************************************************************************/

void updateFanInFanOut(NODE* graph, NODE* graphDup, int max, int count, int newNodes[]) {
    int i;
    int changeFin = 0;
    int changeFot = 0;
    LIST* temp;
    for (i = 1; i <= count; i++)
    {
        if (graphDup[i].Type == 0) //skip unknown nodes
        {
            continue;
        } else
        {
            //update fanin list
            for (temp = graphDup[i].Fin; temp != NULL; temp = temp->next) {
                if (changeFin == 0) { //check if the nodes are original or duplicate
                    temp->id = newNodes[temp->id];

                } else {
                    if (graph[temp->id].Type == 1) { //skip primary inputs
                        temp->id = newNodes[temp->id];
                    } else {
                        temp->id = newNodes[temp->id] + 1;   //update duplicate nodes               
                    }
                }
            }

            //switch between the original and duplicate nodes

            if (changeFin == 1) {
                changeFin = 0;
            } else {
                changeFin = 1;
            }

            //update fanout list            
            for (temp = graphDup[i].Fot; temp != NULL; temp = temp->next) {
                if (changeFot == 0) //check if the nodes are original or duplicate
                {
                    temp->id = newNodes[temp->id];
                } else {
                    temp->id = newNodes[temp->id] + 1; //update duplicate nodes
                }
            }
            if (graphDup[i].Type != 1) { //skip primary inputs
                if (changeFot == 1) //switch between the original and duplicate nodes
                {
                    changeFot = 0;
                } else {
                    changeFot = 1;
                }
            } else {
                changeFot = 0;
            }
        }
    }
}
// end of updateFanInFanOut

/***************************************************************************************************************************
 * deep copy of a node
 * ****************************************************************************************************************************/

void deepCopyNode(NODE* dest, NODE* src) {
    //deep copy fot
    if (src->Fot != NULL) {
        dest->Fot = malloc(sizeof(LIST));
        LIST* srcTemp = src->Fot;
        LIST* destTemp = dest->Fot;
        while (srcTemp != NULL) {
            destTemp->id = srcTemp->id;
            if (srcTemp->next != NULL) {
                destTemp->next = malloc(sizeof(LIST));
            } else {
                destTemp->next = NULL;
            }
            srcTemp = srcTemp->next;
            destTemp = destTemp->next;
        }
    } else {
        dest->Fot = NULL;
    }
    //deep copy fin
    if (src->Fin != NULL) {
        dest->Fin = malloc(sizeof(LIST));
        LIST* srcTemp = src->Fin;
        LIST* destTemp = dest->Fin;
        while (srcTemp != NULL) {
            destTemp->id = srcTemp->id;
            if (srcTemp->next != NULL) {
                destTemp->next = malloc(sizeof(LIST));
            } else {
                destTemp->next = NULL;
            }
            srcTemp = srcTemp->next;
            destTemp = destTemp->next;
        }
    } else {
        dest->Fin = NULL;
    }
}
// end of deepCopyNode

/***************************************************************************************************************************
 * Add xor components to the graph
 * ****************************************************************************************************************************/

void addXorComponents(NODE* graphDup, int count, int xorCount, int final) {
    //add or gate PO to combine the xor gates
    graphDup[final].Type = AssignType("OR");
 
    int j;
    for(j = 1; j <= count; j++){
        //add xor gates
        if(graphDup[j].Po == 1){
           
            graphDup[count+1].Type = AssignType("XOR");
            InsertList(&graphDup[count+1].Fin, j ); //fanin of xor gate from original node
            InsertList(&graphDup[count+1].Fin, j+1); //fanin of xor gate from duplicate node
            InsertList(&graphDup[count+1].Fot, final); //fanout of xor gate to or gate

            graphDup[j].Po = 0; //set primary output to 0
            graphDup[j+1].Po = 0;
            InsertList(&graphDup[j].Fot, count+1); //fanout of original node to xor gate
            InsertList(&graphDup[j+1].Fot, count+1); //fanout of duplicate node to xor gate

            InsertList(&graphDup[final].Fin, count+1); //fanin of or gate from xor gate

            count = count + 1; //increment count

            j = j + 1;
        }
    }
    graphDup[final].Po = 1; //set primary output to 1 in or gate
}
// end of addXorComponents

/***************************************************************************************************************************
 * fault injection
 * ****************************************************************************************************************************/

void faultInjection(NODE* graphDup, NODE* graph, int count, int max, int oldTonew[], char* fname){
    int i;

   // Create a directory with the given circuit name
    struct stat st = {0};
    if (stat(fname, &st) == -1) {
        mkdir(fname, 0700); // Create directory with read/write/execute permissions for the owner
    }

    for (i = 1; i <= max; i++)
    {
        int newId = oldTonew[i]; //get the new id of the node
        if (graphDup[newId].Type == 1){ //skip primary inputs
            continue;
        } else if (graphDup[newId].Type == 0){ //skip unknown nodes
        
            continue;
        } else {
            char fileName[Mfnam]; // Buffer to hold the file name
            
            if (graphDup[newId].Type == 9){ //convert inverter to buffer 

                // create a graph with buffer add error to the node
                graphDup[newId].Type = 8;

                // write bechmark file with error
                sprintf(fileName, "%s/%s_%dNOT_to_BUFF.bench", fname, fname, newId);
                FILE* fbench = fopen(fileName, "w");
                writeBenchmarkFile(max, graphDup, fbench);
                fclose(fbench);

                graphDup[newId].Type = 9; // change the gragh to error free
            } else if (graphDup[newId].Type == 8){ //convert buffer to inverter
               
                graphDup[newId].Type = 9; // add error to the node

                sprintf(fileName, "%s/%s_%dBUFF_to_NOT.bench",fname, fname, newId);
                FILE* fbench = fopen(fileName, "w");
                writeBenchmarkFile(max, graphDup, fbench);
                fclose(fbench);

                graphDup[newId].Type = 8; // change the gragh to error free
            } else {
                // create file for each node types
                convertType(graphDup, max, graphDup[newId].Type, newId, fname);
            }
        }
    }
}
// end of faultInjection

/***************************************************************************************************************************
 * convert the type of the node
 * ****************************************************************************************************************************/

void convertType(NODE* graph, int max, int typeId, int i, char* fname) {
    int j;
    char* type = typeToString(typeId);
    const char* types[] = { "AND", "NAND", "OR", "NOR", "XOR", "XNOR" };
    int len = 6 ;
    printf("len %d\n", len);
    char fileName[Mfnam]; // Buffer to hold the file name

    for( j = 0; j < len; j++){
        if (type == types[j]){
            continue;
        } else {
            sprintf(fileName, "%s/%s_%d_%sto_%s.bench", fname,fname, i, type, types[j]);
            FILE* fbench = fopen(fileName, "w");
            graph[i].Type = AssignType(types[j]); // add error to the node

            // write bechmark file with error
            writeBenchmarkFile(max, graph, fbench);
            fclose(fbench);
            graph[i].Type = AssignType(type); // change the gragh to error free
        } 
    }
}

// end of convertType

/***************************************************************************************************************************
 * convert the type of the node to string
 * ****************************************************************************************************************************/

const char* typeToString(int type) {
    switch (type) {
        case INPT: return "INPT";
        case BUFF: return "BUFF";
        case NOT: return "NOT";
        case AND: return "AND";
        case NAND: return "NAND";
        case OR: return "OR";
        case NOR: return "NOR";
        case XOR: return "XOR";
        case XNOR: return "XNOR";
        case FROM: return "FROM";
        default: return "UNKNOWN";
    }
}
// end of typeToString

/***************************************************************************************************************************
 * create fault file
 * ****************************************************************************************************************************/

void createFaultFile(int max, char* fname) {
    int i;
    char fileName[Mfnam]; // Buffer to hold the file name
   
    sprintf(fileName, "%s/%s.fault", fname, fname);
    FILE* fbench = fopen(fileName, "w");

    fprintf(fbench, "%d /0\n", max);
    fclose(fbench);
   
}
// end of createFaultFile

/***************************************************************************************************************************
 * execute atlanta
 * ****************************************************************************************************************************/

void executeAltanta(char* fname, char* benchName) {
    char* benchFile [Mfnam];
    sprintf(benchFile, "%s/%s", fname, benchName);
    char* faultFile [Mfnam];
    sprintf(faultFile, "%s/%s.fault", fname, fname);
    char* outputFile [Mfnam];
    //remove the .bench extension
    benchName[strlen(benchName) - 6] = '\0';
    sprintf(outputFile, "%s/%s.test", fname, benchName);

    //build the command to execute atlanta > /path/to/output_directory/output.test
    char command[256];                  
    snprintf(command, sizeof(command), "/opt/net/apps/atalanta/atalanta -A -f %s -t %s %s", faultFile, outputFile, benchFile);
    printf("command = %s\n", command);
    //pass the command to the system
    system(command);

}
// end of executeAltanta
/***************************************************************************************************************************
 * process bench files
 * ****************************************************************************************************************************/

void processBenchFiles(char* fname) {
    struct dirent* entry;
    DIR* dp = opendir(fname);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dp))) {
        //read onyl .bench files
        if (strstr(entry->d_name, ".bench") != NULL) {
            
            //execute atlanta
            executeAltanta(fname, entry->d_name);
        }
    }
    closedir(dp);
}
// end of processBenchFiles

/***************************************************************************************************************************
 * read test file
 * ****************************************************************************************************************************/

int readTestFile( FILE* ftest, PatternData* patterns) {
    //read the .test file and store the information in  array structure

	char line[Mlin];
    int patternsStart = 0;
    int numPatterns = 0; //number of patterns

	
	while (fgets(line, Mlin, ftest)){

		if (line[0] == '*') {//skip comments
			continue;
		} else if (line[0] == '\n'){ //skip empty lines
            continue;
        }else if (strchr(line, '/') != NULL){ // fault/stuk at 0/1
            patternsStart = 1;
            continue;
        }else if(patternsStart == 1){ //read the test patterns    
            char pattern[Mlin];
            int faultFreeVal;
            sscanf(line, "  %*d: %s %d", pattern, &faultFreeVal); // Extract the pattern and fault-free value
            numPatterns = numPatterns + 1;
    
            patterns[numPatterns].faultFreeVal = faultFreeVal;
            strcpy((patterns)[numPatterns].pattern, pattern);
  
        } else {
            continue;
            //store the information in the array
        }	
	}
  
    return numPatterns;
    //read the .test file and store the information in  vector structure
}
// end of readTestFile

/***************************************************************************************************************************
 * create test pattern file
 * ****************************************************************************************************************************/

void processTestfiles(char* fname,  int maxPat){

    char fpatternname[Mfnam]; // Buffer to hold the file name
    sprintf(fpatternname, "%s/%s_rand%d.pattern", fname, fname,maxPat);
    FILE* ftestPattern = fopen(fpatternname, "w");

    struct dirent* entry;
    DIR* dp = opendir(fname);

    if (dp == NULL) {
        perror("opendir");
        return;
    }
    int itr = 0;

    while ((entry = readdir(dp))) { //read all files in the directory
        
            char* ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".test") == 0) { //read only .test files
                char filePath[Mfnam]; // Buffer to hold the file path
                sprintf(filePath, "%s/%s", fname, entry->d_name);
 
                FILE* ftest = fopen(filePath, "r"); //open the file
                if (ftest == NULL) {
                    perror("fopen");
                    return;
                }
                PatternData pattern[MAX_PATTERNS];
                int patternCount = readTestFile(ftest, pattern);
                fclose(ftest);

                if(patternCount!=0){ //if no patterns in the file skip increasing the itr
                    itr = itr + 1;
                }
                int i;
                fprintf(ftestPattern, "%s\n", entry->d_name); //write the file name to the test pattern file
                
                if(patternCount == 0){ //if no patterns in the file skip writing the patterns
                    continue;
                }else if(patternCount < maxPat){ //if the number of patterns in the file is less than the required number of patterns         
                    for (i = 1; i <= patternCount; i++){
                        fprintf(ftestPattern, "%s %d\n", pattern[i].pattern, pattern[i].faultFreeVal);
                    }
                }else{
                    int randomIndex[maxPat];
                    generateUniqueRandomNumbers(randomIndex, patternCount, maxPat);
                    for (i = 0; i < maxPat; i++){
                        fprintf(ftestPattern, "%s %d\n", pattern[randomIndex[i]].pattern, pattern[randomIndex[i]].faultFreeVal);
                    }
                }    
                
                }
                fprintf(ftestPattern, "\n");
                
            if(itr ==MAX_RND_PATTERNS){ //break after reading 500 test files
                break;
            }
        }
        fclose(ftestPattern);
}
// end of createTestPatternFile

/***************************************************************************************************************************
 * create test patterns
 * ****************************************************************************************************************************/
void createTestPatterns(char* fname) {
    //create a directory with the given circuit name
    
    int maxPat[] = {1, 2, 3, 4};
    int i;
    for ( i = 0; i < 4; i++){
        processTestfiles(fname, maxPat[i]);
    }
}

int isInArray(int num, int *array, int size) {
    int i;
    for ( i = 0; i < size; i++) {
        if (array[i] == num) {
            return 1;
        }
    }
    return 0;
}

void generateUniqueRandomNumbers(int *randomIndex, int patternCount, int size) {
    int i;
    for (i = 0; i < size; i++) {
        int randomNum;
        do {
            randomNum = rand() % patternCount + 1;
        } while (isInArray(randomNum, randomIndex, i));
        randomIndex[i] = randomNum;
    }
}