#include "usrList.h"

UsrNode *mkUsrNode()
{
	UsrNode *node = (UsrNode *)malloc(sizeof(UsrNode));
	if (NULL == node)
	{
		printf("malloc usr node failure\n");
		exit(EXIT_FAILURE);
	}
	memset(node, 0, sizeof(UsrNode));
	return node;
}

UsrList *mkUsrList()
{
	UsrList *list = (UsrList *)malloc(sizeof(UsrList));
	if (NULL == list)
	{
		printf("malloc usr list failure\n");
		exit(EXIT_FAILURE);
	}
	memset(list, 0, sizeof(UsrList));
	list->uiBaseId = 10000;

	return list;
}

void insertUsrList(UsrList *list, UsrNode *node)
{
	if (NULL == list || NULL == node)
	{
		return;
	}		
	node->pNext = list->pFirstNode;
	list->pFirstNode = node;
	list->uiLen++;
}

void display(const UsrList *list)
{
	if (NULL != list)
	{
		UsrNode *node = list->pFirstNode;
		while (NULL != node)
		{
			printf("id:%d, pd:%s\n", node->uiId, node->caPwd);
			node = node->pNext;
		}
	}
}

UsrNode *findById(uint uiId, const UsrList *list)
{
	UsrNode *node = list->pFirstNode;
   	while (NULL != node)
	{
		if (uiId == node->uiId)
		{
			break;
		}
		node = node->pNext;
	}	

	return node;
}
