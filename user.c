#include "graph.h"
#include "user.h"

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
    for (LIST* temp = Fin; temp != NULL; temp = temp->next)
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

int createDuplicateGraph(NODE* graph, NODE* graphDup, int max) {

    int i;
    int count = 0;
    int oldToNewNodes[max + 1]; // Declare the array

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
        if (graph[i].Type == 1)
        {
            graphDup[j] = graph[i];
            oldToNewNodes[i] = j;
            count = count + 1;
            j++;
        } else if (graph[i].Type == 0)
        {
            continue;
        } else {
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

    //connect xor gates to the new primary outputs


    return count;
}
// end of createDuplicateGraph

/***************************************************************************************************************************
 * update fanin and fanout lists
 * ****************************************************************************************************************************/

void updateFanInFanOut(NODE* graph, NODE* graphDup, int max, int count, int newNodes[]) {
    int i;
    int changeFin = 0;
    int changeFot = 0;
    for (i = 1; i <= count; i++)
    {
        if (graphDup[i].Type == 0) //skip unknown nodes
        {
            continue;
        } else
        {
            //update fanin list
            for (LIST* temp = graphDup[i].Fin; temp != NULL; temp = temp->next) {
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
            for (LIST* temp = graphDup[i].Fot; temp != NULL; temp = temp->next) {
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