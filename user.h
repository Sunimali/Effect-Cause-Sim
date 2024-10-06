/***************************************************************************************************************************
Header Files
****************************************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <time.h>
#include <complex.h>
#include <assert.h>
#include <limits.h>

/***************************************************************************************************************************
 * Constants
 * ****************************************************************************************************************************/
#define MAX_PATTERNS 100
#define MAX_RND_PATTERNS 500

typedef struct {
    char pattern[Mlin];
    int faultFreeVal;
} PatternData;


/***************************************************************************************************************************
 * Function Declarations
 * ****************************************************************************************************************************/
int createDuplicateGraph(NODE *graph, NODE *graphDup, int max, int newNodes[]);
int writeBenchmarkFile(int max, NODE *graph, FILE *fbench);
void printNameFromTypeId(int type, FILE *fbench);
void printFanInList(LIST *Fin, FILE *fbench);
void updateFanInFanOut(NODE *graph, NODE *graphDup, int max, int count, int newNodes[]);
void deepCopyNode(NODE* dest, NODE* src);
void addXorComponents(NODE* graphDup, int count, int xorCount, int final) ;
void faultInjection(NODE* graphDup, NODE* graph, int count, int max, int oldTonew[], char* fname);
void convertType(NODE* graph, int max, int typeId, int i, char* fname);
const char* typeToString(int type);
void createFaultFile(int max, char* fname);
void executeAltanta(char* fname, char* benchName);
void processBenchFiles(char* fname);
int readTestFile( FILE* ftest, PatternData* patterns);
void processTestfiles(char* fname,  int maxPat);
void createTestPatterns(char* fname);
void printTestPatternsPerFault(FILE* ftestPattern,PatternData* patterns,  int patternCount, int maxPat);
