#include "graph.h"
#include "user.h"

/***************************************************************************************************************************
 * write netlist into a BenchmarkFile
 * ****************************************************************************************************************************/

int writeBenchmarkFile(int max, NODE *graph, FILE *fbench)
{
    int i;
    fprintf(fbench, "#\n#\n#\n#\n\n");
    for (i = 1; i <= max; i++)  // write primary inputs
    {
        if(graph[i].Type == 1)
        {
            fprintf(fbench, "INPUT(%d)\n", i);
        }
 
    }
    fprintf(fbench, "\n");

    for (i = 1; i <= max; i++) 
    {
        if(graph[i].Po == 1) // write primary outputs
        {
            fprintf(fbench, "OUTPUT(%d)\n", i);
        }      
        
    }
    fprintf(fbench, "\n");
    
    for (i = 1; i <= max; i++) // write the rest of the nodes
    {
        if(graph[i].Type != 0  && graph[i].Type != 1) //skip primary inputs & unknown nodes
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
void printNameFromTypeId(int type, FILE *fbench)
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
       fprintf(fbench,"BUFF");
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

void printFanInList(LIST *Fin, FILE *fbench){
    for(LIST *temp = Fin; temp != NULL; temp = temp->next)
    {
        fprintf(fbench, "%d", temp->id);
        if(temp->next != NULL)
        {
            fprintf(fbench, ",");
        }
    }
}
// end of printFanInList