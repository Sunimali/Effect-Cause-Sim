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
int createDuplicateGraph(NODE *graph, NODE *graphDup);
int writeBenchmarkFile(int max, NODE *graph, FILE *fbench);
void printNameFromTypeId(int type, FILE *fbench);
void printFanInList(LIST *Fin, FILE *fbench);
