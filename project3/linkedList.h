/*
	Author: Mengwen Li (mli2)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

typedef struct node
{
	char* name;
	struct node* next;
}listNode;

typedef listNode* pNode;

void initList(pNode* head, pNode* tail);
void addNode(pNode* head, pNode* tail, char* fileName);
void travList(pNode head);

