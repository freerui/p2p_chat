#ifndef USR_LIST_H
#define USR_LIST_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef unsigned int uint;
#define PWD_LEN 32

typedef struct UsrNode
{
	uint uiId;
	char caPwd[PWD_LEN];
	int sockfd;
	short iIsOnline;
	struct UsrNode *pNext;

}UsrNode;

typedef struct UsrList
{
	uint uiLen;
	uint uiBaseId;
	UsrNode *pFirstNode;
}UsrList;

UsrNode *mkUsrNode();
UsrList *mkUsrList();
void insertUsrList(UsrList *list, UsrNode *node);
void display(const UsrList *list);
UsrNode *findById(uint uiId, const UsrList *list);

#endif
