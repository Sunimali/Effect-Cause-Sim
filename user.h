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
 * Function Declarations
 * ****************************************************************************************************************************/
int createDuplicateGraph(NODE *graph, NODE *graphDup, int max);
int writeBenchmarkFile(int max, NODE *graph, FILE *fbench);
void printNameFromTypeId(int type, FILE *fbench);
void printFanInList(LIST *Fin, FILE *fbench);
void updateFanInFanOut(NODE *graph, NODE *graphDup, int max, int count, int newNodes[]);
void deepCopyNode(NODE* dest, NODE* src);
void addXorComponents(NODE* graphDup, int count, int xorCount, int final) ;