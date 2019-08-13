#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include "usrList.h"
#include "protocol.h"
#include "file.h"

UsrList *g_pUsrList = NULL;

int mkSocket()
{
	int sockfd = -1;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	return sockfd;
}

void mkBind(int sockfd)
{
	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servAddr.sin_port = htons(8888);
	bzero(servAddr.sin_zero, 0);

	int ret = -1;
	ret = bind(sockfd, (struct sockaddr *)&servAddr
				  , sizeof(servAddr));
	if (-1 == ret)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}
}

void mkListen(int sockfd)
{
	int ret = -1;
	ret = listen(sockfd, 20);
	if (-1 == ret)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
}

void handleClientRegistRequest(int sockfd, PDU *pdu)
{
	if (NULL == pdu)
	{
		return;
	}

	//生成节点保存注册的用户信息并将其放入链表中
	UsrNode *node = mkUsrNode();
	node->uiId = g_pUsrList->uiBaseId;
	g_pUsrList->uiBaseId++;
	strncpy(node->caPwd, pdu->caMsg, pdu->uiMsgLen);
	node->sockfd = -1; //登入成功之后保存其sockfd
	insertUsrList(g_pUsrList, node);

	//将链表信息保存到文件中
	saveUsrListToFile(g_pUsrList);

	//生成注册回复信息发送给客户端
	PDU *respdu = mkPDU(strlen(REGIST_OK));
	respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
	respdu->uiTo = node->uiId;
	strncpy(respdu->caMsg, REGIST_OK, strlen(REGIST_OK));
	sendPDU(sockfd, respdu);
	free(respdu);
	respdu = NULL;
}

void handleClientLoginRequest(int sockfd, PDU *pdu)
{
	if (NULL == pdu)
	{
		return;
	}
	UsrNode *node = findById(pdu->uiFrom, g_pUsrList);
	PDU *respdu = NULL;
	if (NULL != node)
	{
		if (0 == strcmp(node->caPwd, pdu->caMsg))
		{
			respdu = mkPDU(strlen(LOGIN_OK)+1);
			strcpy(respdu->caMsg, LOGIN_OK);
			node->sockfd = sockfd;
			node->iIsOnline = 1;
		}
		else
		{
			respdu = mkPDU(strlen(PWD_ERROR)+1);
			strcpy(respdu->caMsg, PWD_ERROR);
		}
	}
	else
	{	
		respdu = mkPDU(strlen(NO_USR)+1);
		strcpy(respdu->caMsg, NO_USR);
	}
	respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
	sendPDU(sockfd, respdu);
	free(respdu);
	respdu = NULL;
}
			
void handleClientGetFriendRequest(int sockfd)
{
	uint uiMsgLen = (g_pUsrList->uiLen)*sizeof(uint);
	printf("len = %d\n", g_pUsrList->uiLen);
	PDU *pdu = mkPDU(uiMsgLen);
	UsrNode *node = g_pUsrList->pFirstNode;
	int i = 0;
	while (NULL != node)
	{
		memcpy(pdu->caMsg+i*sizeof(uint)
			   , &(node->uiId), sizeof(uint));
		i++;
		node = node->pNext;
	}
	pdu->uiMsgType = ENUM_MSG_TYPE_GET_FRIEND_RESPOND;
	sendPDU(sockfd, pdu);
	free(pdu);
	pdu = NULL;
}
			
void handleClientPrivateChatRequest(int sockfd, PDU *pdu)
{
	if (NULL == pdu)
	{
		return;
	}
	UsrNode *node = findById(pdu->uiTo, g_pUsrList);
	PDU *respdu = NULL;
	if (NULL == node) //不存在此人
	{
		respdu = mkPDU(strlen(NO_USR)+1);
		strcpy(respdu->caMsg, NO_USR);	
	}
	else
	{
		if (1 == node->iIsOnline)  //在线
		{
			sendPDU(node->sockfd, pdu);
			return;
		}
		else   //不在线
		{
			respdu = mkPDU(strlen(OFFLINE)+1);
			strcpy(respdu->caMsg, OFFLINE);	
		}
	}
	respdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND;
	sendPDU(sockfd, respdu);
	free(respdu);
	respdu = NULL;
}
			
void handleClientGroupChatRequest(int sockfd, PDU *pdu)
{
	if (NULL == pdu)
	{
		return;
	}
	UsrNode *node = g_pUsrList->pFirstNode;
	while (NULL != node)
	{
		if (1 == node->iIsOnline)
		{
			sendPDU(node->sockfd, pdu);
		}
		node = node->pNext;
	}

}

void* handleClient(void *arg)
{
	int sockfd = (int)arg;
	PDU *pdu = NULL;
	while (1)
	{
		pdu = recvPDU(sockfd);
		if (NULL == pdu)
		{
			printf("客户端与服务器断开连接...\n");
			break;
		}
		switch (pdu->uiMsgType)
		{
		case ENUM_MSG_TYPE_REGIST_REQUEST:
			handleClientRegistRequest(sockfd, pdu);
			break;
		case ENUM_MSG_TYPE_LOGIN_REQUEST:
			handleClientLoginRequest(sockfd, pdu);
			break;
		case ENUM_MSG_TYPE_GET_FRIEND_REQUEST:
			handleClientGetFriendRequest(sockfd);
			break;
		case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
			handleClientPrivateChatRequest(sockfd, pdu);
			break;
		case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
			handleClientGroupChatRequest(sockfd, pdu);
			break;
		default:
			break;
		}
		free(pdu);
		pdu = NULL;
	}
}

void work(int sockfd)
{
	struct sockaddr_in clientAddr;
	int iLen = sizeof(clientAddr);
	int clientSockfd = -1;
	pthread_t thr;
	while (1)
	{
		printf("accept client...\n");
		clientSockfd = accept(sockfd
						      , (struct sockaddr *)&clientAddr
							  , &iLen);
		if (-1 == clientSockfd)
		{
			perror("accept");
			continue;
		}
		printf("client sockfd = %d, ip: %s, port: %d\n"
			   , clientSockfd, inet_ntoa(clientAddr.sin_addr)
			   , ntohs(clientAddr.sin_port));	
		pthread_create(&thr, NULL, handleClient
					   , (void *)clientSockfd);
	}
}

int main(void)
{
	g_pUsrList = mkUsrList();	
	getUsrListFromFile(g_pUsrList);

	int sockfd = mkSocket(); //产生socket用于监听客户端的连接
	mkBind(sockfd); //将socket和要监听的ip及port进行绑定
	mkListen(sockfd);
	work(sockfd);

	return 0;
}
