#include "file.h"

int openFile(const char *pathName, int flags)
{
	int fd = open(pathName, flags|O_CREAT, 0664);
	if (-1 == fd)
	{
		perror("open");
		exit(EXIT_FAILURE);
	}
	return fd;
}

void saveUsrListToFile(const UsrList *list)
{
	if (NULL == list)
	{
		return;
	}

	int fd = openFile(USR_INFO, O_WRONLY);

	UsrNode *node = list->pFirstNode;
	while (NULL != node)
	{
		write(fd, node, sizeof(UsrNode));
		node = node->pNext;
	}
}

void getUsrListFromFile(UsrList *list)
{
	if (NULL == list)
	{
		return;
	}
	
	int fd = openFile(USR_INFO, O_RDONLY);

	UsrNode *node = NULL;
	int ret = 0;
	while (1)
	{
		node = mkUsrNode();
		ret = read(fd, node, sizeof(UsrNode));
		if (-1 == ret || 0 == ret)
		{
			free(node);
			break;
		}
		insertUsrList(list, node);
		if (list->uiBaseId < node->uiId)
		{
			list->uiBaseId = node->uiId;
		}
	}
	list->uiBaseId++;
}
