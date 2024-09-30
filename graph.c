#include "graph.h"
/***************************************************************************************************************************
Insert an element "x" at end of LIST "l", if "x" is not already in "l".
****************************************************************************************************************************/
void insertList(LIST** l, int x) {
	LIST* p, * tl;

	if ((p = (LIST*)malloc(sizeof(LIST))) == NULL) {
		printf("LIST: Out of memory\n");
		exit(1);
	} else {
		p->id = x;
		p->next = NULL;
		if (*l == NULL) {
			*l = p;
			return;
		}
		tl = NULL;
		tl = *l;
		while (tl != NULL) {
			if (tl->id == x) {
				break;
			}
			if (tl->next == NULL) {
				tl->next = p;
			}
			tl = tl->next;
		}
	}
	return;
} // end of insertList
/***************************************************************************************************************************
Print the elements in LIST "l"
***************************************************************************************************************************/
void printList(LIST* l) {
	LIST* temp = NULL;

	temp = l;
	while (temp != NULL) {
		printf("%d ", temp->id);
		temp = temp->next;
	}
	return;
} // end of printList
/***************************************************************************************************************************
Free all elements in  LIST "l"
****************************************************************************************************************************/
void freeList(LIST** l) {
	LIST* temp = NULL;

	if (*l == NULL) {
		return;
	}
	temp = (*l);
	while ((*l) != NULL) {
		temp = temp->next;
		free(*l);
		(*l) = temp;
	}
	(*l) = NULL;
	return;
} // end of freeList
/*****************************************************************************************************************************
 Routine to read the Bench Markk(.isc files)
*****************************************************************************************************************************/
int readIsc(FILE* fisc, NODE* graph){
	char c, noty[Mlin], from[Mlin], str1[Mlin], str2[Mlin], name[Mlin], line[Mlin];
	int i, id, fid, Fin, fout, mid = 0;

	// intialize all nodes in graph structure
	for (i = 0; i < Mnod; i++){
		initializeCircuit(graph, i);
	}
	// skip the comment lines
	do
		fgets(line, Mlin, fisc);
	while (line[0] == '*');
	// read line by line
	while (!feof(fisc)){
		// initialize temporary strings
		bzero(noty, strlen(noty));
		bzero(from, strlen(from));
		bzero(str1, strlen(str1));
		bzero(str2, strlen(str2));
		bzero(name, strlen(name));
		// break line into data
		sscanf(line, "%d %s %s %s %s", &id, name, noty, str1, str2);
		// fill in the Typee
		strcpy(graph[id].name, name);
		graph[id].Type = assignType(noty);
		// fill in fanin and fanout numbers
		if (graph[id].Type != FROM){
			fout = atoi(str1);
			Fin = atoi(str2);
		} else{
			Fin = fout = 1;
			strcpy(from, str1);
		}
		if (id > mid){
			mid = id;
		}
		graph[id].Nfo = fout;
		graph[id].Nfi = Fin;
		if (fout == 0){
			graph[id].Po = 1;
		}
		// create fanin and fanout lists
		switch (graph[id].Type){
		case 0:
			printf("readIsc: Error in input file (Node %d)\n", id);
			exit(1);
		case INPT:
			break;
		case AND:
		case NAND:
		case OR:
		case NOR:
		case XOR:
		case XNOR:
		case BUFF:
		case NOT:
			for (i = 1; i <= Fin; i++){
				fscanf(fisc, "%d", &fid);
				insertList(&graph[id].Fin, fid);
				insertList(&graph[fid].Fot, id);
			}
			fscanf(fisc, "\n");
			break;
		case FROM:
			for (i = mid; i > 0; i--){
				if (graph[i].Type != 0){
					if (strcmp(graph[i].name, from) == 0){
						fid = i;
						break;
					}
				}
			}
			insertList(&graph[id].Fin, fid);
			insertList(&graph[fid].Fot, id);
			break;
		} // end case
		bzero(line, strlen(line));
		fgets(line, Mlin, fisc);
	} // end while
	return mid;
} // end of readIsc
/****************************************************************************************************************************
Initialize the paricular memeber of graph structure
****************************************************************************************************************************/
void initializeCircuit(NODE* graph, int num){
	bzero(graph[num].name, Mnam);
	graph[num].Type = graph[num].Nfi = graph[num].Nfo = graph[num].Po = graph[num].Mark = 0;
	graph[num].Cval = graph[num].Fval = 3;
	graph[num].Fin = graph[num].Fot = NULL;
	return;
} // end of initializeCircuit
/****************************************************************************************************************************
Convert (char *) Typee read to (int)
****************************************************************************************************************************/
int assignType(char* Type){

	if ((strcmp(Type, "inpt") == 0) || (strcmp(Type, "INPT") == 0))
		return 1;
	else if ((strcmp(Type, "and") == 0) || (strcmp(Type, "AND") == 0))
		return 2;
	else if ((strcmp(Type, "nand") == 0) || (strcmp(Type, "NAND") == 0))
		return 3;
	else if ((strcmp(Type, "or") == 0) || (strcmp(Type, "OR") == 0))
		return 4;
	else if ((strcmp(Type, "nor") == 0) || (strcmp(Type, "NOR") == 0))
		return 5;
	else if ((strcmp(Type, "xor") == 0) || (strcmp(Type, "XOR") == 0))
		return 6;
	else if ((strcmp(Type, "xnor") == 0) || (strcmp(Type, "XNOR") == 0))
		return 7;
	else if ((strcmp(Type, "buff") == 0) || (strcmp(Type, "BUFF") == 0))
		return 8;
	else if ((strcmp(Type, "not") == 0) || (strcmp(Type, "NOT") == 0))
		return 9;
	else if ((strcmp(Type, "from") == 0) || (strcmp(Type, "FROM") == 0))
		return 10;
	else
		return 0;
} // end of assignType
/****************************************************************************************************************************
Print all members of graph structure(except Type=0) after reading the bench file
*****************************************************************************************************************************/
void printCircuit(NODE* graph, int Max){

	LIST* temp;
	int i;
	printf("\nID\tNAME\tTypeE\tPO\tIN#\tOUT#\tCVAL\tFVAL\tMarkK\tFANIN\tFANOUT\n");
	for (i = 0; i <= Max; i++){

		if (graph[i].Type != 0){
			printf("%d\t%s\t%d\t%d\t%d\t%d\t", i, graph[i].name, graph[i].Type, graph[i].Po, graph[i].Nfi, graph[i].Nfo);
			printf("%d\t%d\t%d\t", graph[i].Cval, graph[i].Fval, graph[i].Mark);
			temp = NULL;
			temp = graph[i].Fin;

			if (temp != NULL){
				printList(temp);
			}
			printf("\t");
			temp = NULL;
			temp = graph[i].Fot;

			if (temp != NULL){
				printList(temp);
			}
			printf("\n");
		}
	}
	return;
} // end of printCircuit
/****************************************************************************************************************************
Free the memory of all member of graph structure
*****************************************************************************************************************************/
void clearCircuit(NODE* graph, int i){
	int num = 0;
	for (num = 0; num < i; num++){

		graph[num].Type = graph[num].Nfi = graph[num].Nfo = graph[num].Po = 0;
		graph[num].Mark = graph[num].Cval = graph[num].Fval = 0;
		if (graph[num].Type != 0){

			bzero(graph[num].name, Mnam);
			if (graph[num].Fin != NULL){
				freeList(&graph[num].Fin);
				graph[num].Fin = NULL;
			}

			if (graph[num].Fot != NULL){
				freeList(&graph[num].Fot);
				graph[num].Fot = NULL;
			}
		}
	}
	return;
} // end of clearCircuit
/*****************************************************************************************************************************
 Routine to read the .vec files
*****************************************************************************************************************************/

/****************************************************************************************************************************/

/*****************************************************************************************************************************
 Routine to read the Bench Markk(.isc files)
*****************************************************************************************************************************/
int readBench(FILE* fbench, NODE* graph){
	int i, id, fid, Fin, fout, mid = 0;
	char line[Mlin];
	// intialize all nodes in graph structure
	for (i = 0; i < Mnod; i++) {
		initializeCircuit(graph, i);
	}

	while (fgets(line, Mlin, fbench)) {

		if (line[0] == '#') {//skip comments
			continue;
		} else if (line[0] == 'I') {//skip input
			sscanf(line, "INPUT(%d)", &id);
			graph[id].Type = assignType("INPT");
		} else if (line[0] == 'O') { //read output
			sscanf(line, "OUTPUT(%d)", &id);
			graph[id].Po = 1;
		} else if (line[0] == '\n') { // skip empty lines
			continue;
		} else {
			char fanInList[Mlin], type[Mlin];
			char token[Mlin];
			bzero(type, strlen(type));
			bzero(fanInList, strlen(fanInList));

			char* o, * c;
			int openIndex, closedIndex;

			sscanf(line, "%d = %[^\n]", &id, token); //get the id and the rest of the line

			//remove the brackets
			o = strchr(token, '(');
			openIndex = (int)(o - token);
			c = strchr(token, ')');
			closedIndex = (int)(c - token);

			strncpy(type, &token[0], openIndex);
			strncpy(fanInList, &token[openIndex + 1], closedIndex - 1);
			graph[id].Type = assignType(type); //assign the type

			char* fanin = strtok(fanInList, ", "); //get the fanin list by splitting the string
			int nFanIn = 0;
			while (fanin != NULL){
				nFanIn++;
				int fanId = atoi(fanin);
				insertList(&graph[id].Fin, fanId); //insert the fanin 
				insertList(&graph[fanId].Fot, id); //insert the fanout
				fanin = strtok(NULL, ", ");
			}
			graph[id].Nfi = nFanIn; //set the number of fanins
		}
		if (id > mid) {
			mid = id; //get the max id and find total number of nodes
		}
	}
	return mid;
} // end of Readbench file