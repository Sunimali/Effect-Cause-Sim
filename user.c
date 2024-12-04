#include "graph.h"
#include "user.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int andGate[3][3] = {
//o   1 xv  	
  {0, 0, 0},
  {0, 1, 2},
  {0, 2, 2}
};

int orGate[3][3] = {
//o   1 xv  	
  {0, 1, 2},
  {1, 1, 1},
  {2, 1, 2}
};

int xorGate[3][3] = {
//o   1 xv  	
  {0, 1, 2},
  {1, 0, 2},
  {2, 2, 2}
};

int notGate[3] = {1, 0 , 2};
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
        initializeCircuit(graphDup, i);
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
             if (graphDup[i].Type != 1) { //skip primary inputs
                if (changeFin == 1) {
                    changeFin = 0;
                } else {
                    changeFin = 1;
                }
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
    graphDup[final].Type = assignType("OR");
 
    int j;
    for(j = 1; j <= count; j++){
        //add xor gates
        if(graphDup[j].Po == 1){

            //handle direct links to primary outputs
            if(graphDup[j].Type == 1){ //skip primary inputs
                graphDup[j].Po = 1;
                continue;
            }else{
           
                graphDup[count+1].Type = assignType("XOR");
                insertList(&graphDup[count+1].Fin, j ); //fanin of xor gate from original node
                insertList(&graphDup[count+1].Fin, j+1); //fanin of xor gate from duplicate node
                insertList(&graphDup[count+1].Fot, final); //fanout of xor gate to or gate

                graphDup[j].Po = 0; //set primary output to 0
                graphDup[j+1].Po = 0;
                insertList(&graphDup[j].Fot, count+1); //fanout of original node to xor gate
                insertList(&graphDup[j+1].Fot, count+1); //fanout of duplicate node to xor gate

                insertList(&graphDup[final].Fin, count+1); //fanin of or gate from xor gate

                count = count + 1; //increment count
            }
            j = j + 1;
        }
    }
    graphDup[final].Po = 1; //set primary output to 1 in or gate
}
// end of addXorComponents

/***************************************************************************************************************************
 * fault injection to duplicate graph
 * ****************************************************************************************************************************/

void faultInjectionToDuplicate(NODE* graphDup, NODE* graph, int count, int max, int oldTonew[], char* fname){
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
// end of faultInjectionToDuplicate

/***************************************************************************************************************************
 * convert the type of the node
 * ****************************************************************************************************************************/

void convertType(NODE* graph, int max, int typeId, int i, char* fname) {
    int j;
    char* type = typeToString(typeId);
    const char* types[] = { "AND", "NAND", "OR", "NOR", "XOR", "XNOR" };
    int len = 6 ;

    char fileName[Mfnam]; // Buffer to hold the file name
    int excludeXOR = 0;

    if(graph[i].Nfi >=3){ 
        excludeXOR = 1;
    }

    for( j = 0; j < len; j++){
        if (type == types[j]){
            continue;
        } else {
            if (excludeXOR == 1 && j >= 4){
                continue;
            } // skip XOR and XNOR if the node has more than 2 fanins(atalanta does not support XOR and XNOR with more than 2 fanins)
            sprintf(fileName, "%s/%s_%d_%sto_%s.bench", fname,fname, i, type, types[j]);
            FILE* fbench = fopen(fileName, "w");
            graph[i].Type = assignType(types[j]); // add error to the node

            // write bechmark file with error
            writeBenchmarkFile(max, graph, fbench);
            fclose(fbench);
            graph[i].Type = assignType(type); // change the gragh to error free
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
                    // /opt/net/apps/atalanta/atalanta 
    snprintf(command, sizeof(command), "/opt/net/apps/atalanta/atalanta -D %d -f %s -t %s %s", MAX_PATTERNS,faultFile, outputFile, benchFile);
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
            
                if(patternCount == 0){ //if no patterns in the file skip writing the patterns
                    continue;
                }else{
                    printTestPatternsPerFault(ftestPattern, pattern, patternCount, maxPat);
                    fprintf(ftestPattern, "\n");
                    itr = itr + 1;
                }    
                
            }
                           
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
// end of generateUniqueRandomNumbers

/***************************************************************************************************************************
 * print test patterns per test file
 * ****************************************************************************************************************************/
void printTestPatternsPerFault(FILE* ftestPattern,PatternData* patterns,  int patternCount, int maxPat){ 
    int i;

    if(patternCount < maxPat){ //if the number of patterns in the file is less than the required number of patterns         
        for (i = 1; i <= patternCount; i++){
            fprintf(ftestPattern, "%s\n", patterns[i].pattern);
        }
    }else{
        int randomIndex[maxPat];
        generateUniqueRandomNumbers(randomIndex, patternCount, maxPat);
        for (i = 0; i < maxPat; i++){
            fprintf(ftestPattern, "%s\n", patterns[randomIndex[i]].pattern);
        }
    }
        
}

//end of printTestPatternsPerFault

/***************************************************************************************************************************
 * Function to read pattern file
 * ****************************************************************************************************************************/

int readPatternFile(FILE* fpat,  char patternList[Mpt][Mlin]) {
    char line[Mlin];
    int tPt = 0; //total patterns
    int i = 0;
    while (fgets(line, Mlin, fpat)) {
        int j;

        //continue if the line is empty
        if (line[0] == '\n') {
            continue;
        }
        
        //remove the new line character
        line[strcspn(line, "\n")] = 0;
        
        //replace x values with 2 in line
        for (j = 0; j < strlen(line); j++) {
            if (line[j] == 'x') {
                line[j] = '2';
            }
        }

    

        if (!isPatternInList(line, patternList, i)) {
            tPt = tPt + 1;
            strcpy(patternList[i], line);
            //replace x with 2 in pattern[i]

            i = i + 1;
        }
    }
    return tPt;
}
//end of readPatternFile

/***************************************************************************************************************************
 * Function to check if a pattern is in the list
 * ****************************************************************************************************************************/

int isPatternInList(const char *pattern,  char patternList[Mpt][Mlin], int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (strcmp(pattern, patternList[i]) == 0) {
            return 1; // Pattern is already in the list
        }
    }
    return 0;
}
//end of isPatternInList

void FaultsSimulator(NODE* node, int max, int tPt, int Npo,  char patternList[Mpt][Mlin], FILE* res) {
    int i;
    int j;
    int typesCount = 11;
    int errorDetected[max][Npo][typesCount]; //initialize the error detected array to 0
    //initialize the error detected array to 0
    for (i = 0; i < max; i++) {
        for (j = 0; j < Npo; j++) {
            int k;
            for (k = 0; k < typesCount; k++) {
                errorDetected[i][j][k] = 0;
            }
        }
    }
    //iterate through all patterns
    for (i = 0; i < tPt; i++) {
        
        int outputNodes[Npo];
        //initialize the output nodes to 0
        for (j = 0; j < Npo; j++) {
            outputNodes[j] = 0;
        }
        //run error free simulation
        simulateLogic(node, Npo, patternList[i], max, outputNodes, 0);
        
        //iterate through all nodes
        for (j = 1; j <= max; j++) {
            if (node[j].Type == 1) { //skip primary inputs
                continue;
            } else if (node[j].Type == 0) { //skip unknown nodes
                continue;
            } else {
                injectFaultToOriginalGraph(node, max, j, node[j].Type, Npo, typesCount,patternList[i], outputNodes, errorDetected);
            }
        }

        //print detected errors per pattern
        printErrorDetected(res, max, Npo, patternList[i], outputNodes, typesCount, errorDetected);

    }

    //  fclose(res);  
}
//end of FaultsSimulator

void injectFaultToOriginalGraph(NODE* graph, int max,int i, int typeId, int Npo, int typesCount, char pattern[Mlin], int outputNodes[Npo],int errorDetected[max][Npo][typesCount]) {
    char* type = typeToString(typeId);
    const char* types[] = { "AND", "NAND", "OR", "NOR", "XOR", "XNOR" };
    int len = 6 ;
    int j;
    int k;
     
    //if buffer conver to not
    if (typeId == 9) {
        graph[i].Type = assignType("NOT");
        simulateLogic(graph, Npo, pattern, max,  outputNodes,1);
        checkDetectedErrors(graph,max,i, Npo, typesCount, outputNodes, errorDetected); //check detected errors

        graph[i].Type = assignType(type); // change the gragh to error free
    } else if (typeId == 8) { //if not convert to buffer
        graph[i].Type = assignType("BUFF");
        simulateLogic(graph, Npo, pattern, max, outputNodes,1);
        checkDetectedErrors(graph,max,i, Npo, typesCount, outputNodes, errorDetected); //check detected errors
        graph[i].Type = assignType(type); // change the gragh to error free
    } else {

        for (j = 0; j < len; j++){
            if (type == types[j]){
                continue;
            } else {
                graph[i].Type = assignType(types[j]); // add error to the node     
                simulateLogic(graph, Npo, pattern, max,  outputNodes,1);
                checkDetectedErrors(graph,max, i, Npo, typesCount, outputNodes, errorDetected); //check detected errors
                graph[i].Type = assignType(type); // change the gragh to error free
            }
        }
    }
}

void printErrorDetected(FILE* res, int max, int Npo, char pattern[Mlin],int outputNodes[Npo], int typesCount, int errorDetected[max][Npo][typesCount]) {
    int j,k,l;
    printf("pattern:%s\n", pattern);
    fprintf(res, "pattern:%s\n", pattern);
  
        for (j = 0; j < Npo; j++) { //iterate through all primary outputs
            fprintf(res, "primary output node id: %d\n", outputNodes[j]);
            printf("primary output node id: %d\n", outputNodes[j]);
            fprintf(res, "fault type:fault node id\n");
            printf("fault type:fault node id\n");
            for(l = 1; l <= max; l++){ //iterate through all nodes
                for (k = 0; k < typesCount; k++) { //iterate through all fault types
                    char* type = typeToString(k);
                    if (errorDetected[l][j][k] == 1) {
                        fprintf(res, "%s%d\n", type, l);   
                    }
                }
            
            }
            fprintf(res, "\n");
        }
        printf("done\n");
}

void checkDetectedErrors(NODE* node, int max, int errorNodeId, int Npo, int typesCount,int outputNodes[Npo], int errorDetected[max][Npo][typesCount]) {
    int i;
    for (i = 0; i < Npo; i++) {
    
        if(node[outputNodes[i]].Cval != node[outputNodes[i]].Fval){
            int type = node[errorNodeId].Type;
            
            errorDetected[errorNodeId][i][type] = 1; //set the error detected to 1
            //clear the error
            node[outputNodes[i]].Cval = node[outputNodes[i]].Fval;
        }
    }   
}

/***************************************************************************************************
 Function to simulate logic
***************************************************************************************************/
                         
 void simulateLogic(NODE * Node, int Npo, char pattern[Mlin], int tGat, int outputNodes[Npo], int error){
		int itr ;
		int curPatternIndex = 0;
        int pocount = 0;
        
            for(itr = 0; itr <= tGat; itr++){ 
                if(Node[itr].Type != 0){  //not an invalid gate
                    switch(Node[itr].Type){
                        case 1: // type input
                            Node[itr].Cval = pattern[curPatternIndex];
                            Node[itr].Fval = pattern[curPatternIndex];
                            curPatternIndex ++;
                            break;
                        case 8: // type  buff
                            Node[itr].Cval = Node[Node[itr].Fin->id].Cval;
                            break;
                        case 9: // type not
                            Node[itr].Cval = notOperation(Node[Node[itr].Fin->id].Cval);
                            break;
                        case 2: //type and
                            Node[itr].Cval = andOperation(Node, (Node[itr].Fin));
                            break;
                        case 3: //type Nand
                            Node[itr].Cval = notOperation(andOperation(Node, (Node[itr].Fin)));
                            break;
                        case 4: // type or
                            Node[itr].Cval = orOperation(Node, (Node[itr].Fin));
                            break;
                        case 5: // type Nor
                            Node[itr].Cval = notOperation(orOperation(Node, (Node[itr].Fin)));
                            break;
                        case 6: // type xor
                            Node[itr].Cval = xorOperation(Node, (Node[itr].Fin));
                            break;
                        case 7: // type xNor
                            Node[itr].Cval = notOperation(xorOperation(Node, (Node[itr].Fin)));
                            break;
                    }
                    if(Node[itr].Po == 1){
                        outputNodes[pocount] = itr;                         
                        //add primary output nodes to a list
                        if(error == 1){
                            int temp = Node[itr].Cval; //swap values once to add error
                            Node[itr].Cval = temp;
                            Node[itr].Cval = Node[itr].Fval;
                            Node[itr].Fval = temp;
                             
                        }else{
                            Node[itr].Fval = Node[itr].Cval; //maintain same value for error and error free
                        }
                        pocount = pocount + 1;
               
                    }
                }
        } 
       
 }

//end of simulateLogic
/****************************************************************************************************************************/

/***************************************************************************************************
 Function to notOperation
***************************************************************************************************/
int notOperation(int input){
	if(input == 0){
        return 1;
    }else{
        return 0;
    }
}
//end of andOperation
/****************************************************************************************************************************/


/***************************************************************************************************
 Function to andOperation
***************************************************************************************************/
int andOperation(NODE * Node, LIST *Cur){
	LIST *tmp=Cur;
	int output=1;

	while(tmp!=NULL){ 
        output = andGate[output][Node[tmp->id].Cval]; // check output by two by two 
		if(output == 0){ // if one pair give out as 0 then stop
			break;
		}
		tmp = tmp->next; 
	} 
	return output;
}
//end of andOperation
/****************************************************************************************************************************/

/***************************************************************************************************
 Function to orOperation
***************************************************************************************************/
int orOperation(NODE * Node, LIST *Cur){
	LIST *tmp=Cur;
	int output=0;


	while(tmp!=NULL){   
		output = orGate[output][Node[tmp->id].Cval]; // check output by two by two
		if(output == 1){ // if one pair give out as 1 then stop
			break;
		}
		tmp = tmp->next; 
	} 
	return output;
}
//end of orOperation
/****************************************************************************************************************************/


/***************************************************************************************************
 Function to xorOperation
***************************************************************************************************/
int xorOperation(NODE * Node, LIST *Cur){
	LIST *tmp=Cur;
    int one = 0;
	int output=1;
 
	while(tmp!=NULL){   
        if(Node[tmp->id].Cval == 1){
            one = one + 1;
        }else if(Node[tmp->id].Cval == 0){
            continue;
        }else{ // if xv
            output = 2;
            return output;
        }        
        tmp = tmp->next; 
    } 
    // check ones count is odd: output is 1
    if(one % 2 == 1){
        output = 1;
    }else{
        output = 0;
    }    
		
	return output;
}
//end of xorOperation
/****************************************************************************************************************************/

//extract the primary outputs fault lists for a pattern
/***************************************************************************************************
 Function to extract the primary outputs fault lists for patterns
***************************************************************************************************/
int extractPrimaryOutputsFaultList(char* fname, int groupSize, char pattern[groupSize][Mlin], char* faults[500][Mlin]) {
    
    char finCopyName[Mfnam]; // Buffer to store the file name
    sprintf(finCopyName, "%s/%s_rand%d.copy", fname, fname, groupSize);
 
    char finName[Mfnam]; // Buffer to store the file name
    sprintf(finName, "%s/%s_rand%d.res", fname, fname, groupSize);
   
    FILE *fin = fopen(finName, "r");
    FILE *fcopy = fopen(finCopyName, "w");

    // printf("Extracting details for patterns\n");


    char line[Mlin];
    int writeBlock = 0;
    int mfaults = 0;

    while (fgets(line, sizeof(line), fin)) { // Iterate through the file line by line

        if (strncmp(line, "pattern:", 8) == 0) { // Check for the start of a pattern block
            writeBlock = 0; // Reset writing for a new block

            int i;
            for (i = 0; i < groupSize; i++) { // Check if this pattern matches any in the given group
                // printf("pattern: %s\n", pattern[i]);
                //pattern: 11021 is line need to extracted 11021
                strcpy(line, line + 8); // Remove the "pattern: " prefix
                
                // printf("line: %s\n", line);
                if (strstr(line, pattern[i])) {
                    writeBlock = 1; // Start writing this block
                    fprintf(fcopy, "%s", line); // Write the pattern line
                    break;
                }
            }
        } else if (writeBlock) { // Continue writing the current block
            if (strncmp(line, "pattern:", 8) == 0) {
                writeBlock = 0; // Stop writing when encountering the next pattern
            } else {
                fprintf(fcopy, "%s", line);
                //create unique fault list
                if (strstr(line, "fault type:fault node id") == NULL && strstr(line, "primary output node id") == NULL) {
                    // printf("fault type:fault node id\n");
                   //check fault in already in list 
                    char trimmedfault[Mlin];
                    strcpy(trimmedfault, line);
                    trimmedfault[strcspn(trimmedfault, "\n")] = 0; // Remove the newline character
                   if (!isFaultInList(trimmedfault, faults, 500)) {
                        strcpy(faults[mfaults], trimmedfault);
                        mfaults = mfaults + 1;
                    }
                }
            }
        }
    }

    fclose(fin);
    fclose(fcopy);

    printf("Details for patterns have been extracted\n");

    return mfaults;
}
//end of extractPrimaryOutputsFaultList

//check fault in already in fault list
/***************************************************************************************************
 Function to check if a fault is already in the fault list
***************************************************************************************************/
int isFaultInList(char* fault, char* faults[500][Mlin], int size) {
    int i;
    for (i = 0; i < size; i++) {
        // printf("faulti: %s\n", faults[i]);
        if (strcmp(fault, faults[i]) == 0) {
            return 1; // Fault is already in the list
        }
    }
    return 0;
}

//end of isFaultInList

//get the intersection of marked faults and current faults block
/***************************************************************************************************
 Function to get the intersection of marked faults and current faults block
***************************************************************************************************/

int keepIntersection(char* markedFaults[500][Mlin], int markedCount, char* currentFaultsBlock[500][Mlin], int currentFaultsBlockCount) {
    //iterate through marked faults and keep only the intersection of marked faults and current faults block
    int i;
    int newMarkedCount = 0;
    for(i = 0; i < markedCount; i++){
        int j;
        for(j = 0; j < currentFaultsBlockCount; j++){
            if(strcmp(markedFaults[i], currentFaultsBlock[j]) == 0){
                //keep the fault in the marked faults list
                strcpy(markedFaults[newMarkedCount], markedFaults[i]);
                newMarkedCount = newMarkedCount + 1;
                break;
            }
        }  
    }
    return newMarkedCount;
}
//end of getIntersection

//remove the intersection of marked faults and unmarked faults
/***************************************************************************************************
 Function to remove the intersection of marked faults and unmarked faults
***************************************************************************************************/

int removeIntersection(char markedFaults[500][Mlin], int markedCount, char unmarkedFaults[500][Mlin], int unmarkedCount) {
    //iterate through marked faults and remove the intersection of marked faults and unmarked faults
    int i;
    int newMarkedCount = 0;
    for(i = 0; i < markedCount; i++){
        int j;
        
        for(j = 0; j < unmarkedCount; j++){
            printf("markedFault: %s unmarkedFaults: %s\n", markedFaults[i], unmarkedFaults[j]);
           
            if(strcmp(markedFaults[i], unmarkedFaults[j]) == 0){
                //remove the fault from the marked faults list
                
                break;
            }
        }  
        if(j == unmarkedCount){
            //keep the fault in the marked faults list
            // strcpy(markedFaults[newMarkedCount], markedFaults[i]);
            newMarkedCount = newMarkedCount + 1;
        }
    }
    return newMarkedCount;
}
//end of removeIntersection

//process current faults block
/***************************************************************************************************
 Function to process current faults block
***************************************************************************************************/
// int processCurrentFaultsBlock(char* currentFaultsBlock[500][Mlin], int currentFaultsBlockCount, char* markedFaults[500][Mlin], int markedCount, char* unmarkedFaults[500][Mlin], int unmarkedCount) {
//     //process current block
//     if(marked){
//         //add current fault block to marked faults
//         int i;int j;
//         //keep only the intersection of marked faults and current faults block in the marked faults list
//         if(markedCount > 0){ //already has faults in marked list
//             markedCount = keepIntersection(markedFaults, markedCount, currentFaultsBlock, currentFaultsBlockCount);
//         }else{
//             //add current fault block to marked faults
//             for (i = 0; i < currentFaultsBlockCount; i++) {
//                 strcpy(markedFaults[markedCount], currentFaultsBlock[i]);
//                 markedCount = markedCount + 1;
//             }
//         }
//         return markedCount;      
//     }else{
//         // add to unmarked faults unique faults //update current umarked list
//         for (i = 0; i < currentFaultsBlockCount; i++) {
//             if (!isFaultInList(currentFaultsBlock[i], unmarkedFaults, 500)) {
//                 strcpy(unmarkedFaults[unmarkedCount], currentFaultsBlock[i]);
//                 unmarkedCount = unmarkedCount + 1;
//             }
//         }
        
//         return unmarkedCount;
//     }
// }



//maintain marked and unmarked fault list
/***************************************************************************************************
 Function to maintain marked and unmarked fault list
***************************************************************************************************/
int maintainMarkedUnmarkedFaultList(char* fault, char* fname, int groupSize){
    //read the faults in copy file and maintain marked and unmarked fault list
    char finCopyName[Mfnam]; // Buffer to store the file name
    sprintf(finCopyName, "%s/%s_rand%d.copy", fname, fname, groupSize);
    
    FILE *fcopy = fopen(finCopyName, "r");

    char line[Mlin];
    int listBlock = 0;
    char markedFaults[500][Mlin];
    char unmarkedFaults[500][Mlin];
    char currentFaultsBlock[500][Mlin];


    int markedCount = 0;
    int unmarkedCount = 0;
    int currentFaultsBlockCount = 0;

    int marked = 0;

    while (fgets(line, sizeof(line), fcopy)) { // Iterate through the file line by line

        // printf("line: %s\n", line);
        // printf("marked: %d\n", marked);
        
          // Remove trailing newline
        line[strcspn(line, "\n")] = '\0';
        char* trimmedLine = line;

        // if(trimmedLine == NULL){
        //     continue;
        // }

        if(strstr(trimmedLine, "fault type:fault node id") != NULL){
            printf("fault type:fault node id\n");
            listBlock = 1;
            continue;
        }
        if (listBlock ) { // Continue writing the current block
            //if trimmed line is empty
            // if (strncmp(trimmedLine, "primary output", 14) == 0) {

            if (trimmedLine[0] == '\0') {
                printf("currentFaultsBlockCount: %d\n", currentFaultsBlockCount);

                //process current block
                if(marked){
                    //add current fault block to marked faults
                    int i;int j;
                    //keep only the intersection of marked faults and current faults block in the marked faults list
                    if(markedCount > 0){ //already has faults in marked list
                        markedCount = keepIntersection(markedFaults, markedCount, currentFaultsBlock, currentFaultsBlockCount);
                    }else{
                        //add current fault block to marked faults
                        for (i = 0; i < currentFaultsBlockCount; i++) {
                            strcpy(markedFaults[markedCount], currentFaultsBlock[i]);
                            markedCount = markedCount + 1;
                        }
                    }      
                }else{
                    int i;
                    // add to unmarked faults unique faults //update current umarked list
                    for (i = 0; i < currentFaultsBlockCount; i++) {
                    
                        if (isFaultInList(currentFaultsBlock[i], unmarkedFaults, unmarkedCount)) {
                            printf(currentFaultsBlock[i]);
                            //print unmarked faults
                            

                        }

                        if (!isFaultInList(currentFaultsBlock[i], unmarkedFaults, unmarkedCount)) {
                            strcpy(unmarkedFaults[unmarkedCount], currentFaultsBlock[i]);
                            unmarkedCount = unmarkedCount + 1;
                        }
                    }
                    //check if there is any intersection between marked and unmarked faults in last only
                }


                listBlock = 0; //reset block
                marked = 0; //reset marked
                currentFaultsBlockCount = 0; //reset current fault block count
                continue;
            }else{
                //check trimmed line is equal to fault
                strcpy(currentFaultsBlock[currentFaultsBlockCount], trimmedLine);
                currentFaultsBlockCount = currentFaultsBlockCount + 1; 
                if (strcmp(trimmedLine, fault) == 0) { //marked list
                    printf("marked\n");
                    marked = 1;  
                } 
            }
          
        }

    }

    printf("Marked count: %d\n", markedCount);
    printf("Unmarked count: %d\n", unmarkedCount);

    //print unmarked faults
    int i;
    for (i = 0; i < unmarkedCount; i++) {
        printf("Unmarked fault: %s\n", unmarkedFaults[i]);
    }

    //print marked faults
    for (i = 0; i < markedCount; i++) {
        printf("Marked fault: %s\n", markedFaults[i]);
    }

    //remove the intersection of marked faults and unmarked faults
    markedCount = removeIntersection(markedFaults, markedCount, unmarkedFaults, unmarkedCount); //this is the resulution
    printf("Marked count after removing intersection: %d\n", markedCount);
    return markedCount;
}

//end of maintainMarkedUnmarkedFaultList

/***************************************************************************************************
 Function read test set file
***************************************************************************************************/

int readTestSetFile(FILE* ftest, char* fname, int groupSize,FILE* fresoultion){
    char line[Mlin];
    int tPt = 0; //total patterns
    int i = 0;
    char pattern[groupSize][Mlin];
    printf("Reading test set file\n");
    printf("Group size: %d\n", groupSize);
    printf("File name: %s\n", fname);

    while (fgets(line, Mlin, ftest)) {

        //remove trailing newline
        line[strcspn(line, "\n")] = 0;
      
        //add group size patterns to the array
        if (line[0] == '\n') {
            continue;
        }
        // if line has x replace it with 2
        int j;
        for (j = 0; j < strlen(line); j++) {
            if (line[j] == 'x') {
                line[j] = '2';
            }
        }
        strcpy(pattern[i], line);
        fprintf(fresoultion, "%s\n", line);
        i = i + 1;
    
        if (i == groupSize) {
            //process the group of patterns
            char* faults[500][Mlin];
            int mfaults = extractPrimaryOutputsFaultList(fname, groupSize, pattern, faults);

            printf("mfaulsts: %d\n", mfaults);

            //do this multiple times
            if (mfaults == 0) {
                i = 0;
                continue;
            }
            int quarter = mfaults;
            if(mfaults > 4){
                quarter = mfaults / 4;
            }
            if (quarter > 500) {
               // only do this for 500 faults
                quarter = 500;
               
            }
            fprintf(fresoultion, "\n");
            printf("quarter: %d\n", quarter);
            //iterate this for quarter times
            fprintf(fresoultion, "Faults resolution\n");
            pickRandomFaults(fresoultion, quarter, faults, mfaults, fname, groupSize);
            fprintf(fresoultion, "\n");
            i = 0;
        }
    } 
}
//end of readTestSetFile

void pickRandomFaults(FILE* fresolution, int quarter, char* faults[500][Mlin], int mfaults, char* fname, int groupSize) {
    int j;
    for (j = 0; j < quarter; j++) {
                //pick unique random fault from the list in each iteration
                int randomIndex = rand() % mfaults;
                char* fault = faults[randomIndex];
                // char* fault = "AND10";
                printf("fault: %s\n", fault);
               
                int resolution = maintainMarkedUnmarkedFaultList(fault, fname, groupSize);
                printf("resolution: %d\n", resolution);

                //write the resolution of the fault
                fprintf(fresolution, "%s : %d\n", fault, resolution);

                faults[randomIndex][Mlin] = faults[mfaults - 1][Mlin]; //swap the last element with the random index
                mfaults = mfaults - 1;   //reduce the fault count by 1 remove the last element
               
    }
}

