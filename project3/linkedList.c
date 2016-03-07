/*
	Author: Mengwen Li (mli2)
*/
#include "linkedList.h"

void initList(pNode* head, pNode* tail)
{
	*head = *tail = NULL;
	return;
}

void addNode(pNode* head, pNode* tail, char* fileName)
{
	pNode newNode = malloc(sizeof(listNode));
	if(newNode == NULL)
	{
		printf("malloc() failed.\n");
		exit(-1);
	}
	newNode -> name = (char*)malloc(strlen(fileName) * sizeof(char));
	if(newNode -> name == NULL)
	{
		printf("malloc() failed.\n");
		exit(-1);
	}
	strcpy(newNode -> name, fileName);
	newNode -> next = NULL;
	if(*head == NULL)
	{
		*head = newNode;
		*tail = *head;
		return;
	}
	(*tail) -> next = newNode;
	*tail = (*tail) -> next;
	return;
}

void travList(pNode head)
{
	while(head != NULL)
	{
		printf("%s\n", head -> name);
		head = head -> next;
	}
	return;
}

