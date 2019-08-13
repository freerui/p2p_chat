#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "protocol.h"

unsigned int g_uiId = 0;

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

void connectToServer(int sockfd)
{
	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servAddr.sin_port = htons(8888);
	bzero(servAddr.sin_zero, 0);

	int ret = -1;
	ret = connect(sockfd, (struct sockaddr *)&servAddr
				  , sizeof(servAddr));
	if (-1 == ret)
	{
		perror("connect");
		exit(EXIT_FAILURE);
	}
	printf("连接服务器成功...\n");
}

void getStrFromSTDIN(char *pData, int size)
{
	if (NULL == pData || size <= 0)
	{
		return;
	}
	read(STDIN_FILENO, pData, size);
	char *p = strchr(pData, '\n');
	if (NULL != p)
	{
		*p = '\0';
	}
	else
	{
		p[size-1] = '\0';
		while ('\n' != getchar())
		{
		}
	}
}

void regist(int sockfd)
{
	char caPwd[32] = {'\0'};
	printf("请输入注册所需密码:\n");
	getStrFromSTDIN(caPwd, 32);

	//产生注册请求信息发送给服务器
	PDU *pdu = mkPDU(strlen(caPwd));
	pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
	strncpy(pdu->caMsg, caPwd, strlen(caPwd));
	sendPDU(sockfd, pdu);
	free(pdu);
	pdu = NULL;

	//接收服务器的注册回复信息
	pdu = recvPDU(sockfd);
	if (ENUM_MSG_TYPE_REGIST_RESPOND == pdu->uiMsgType)
	{
		if (0 == strncmp(REGIST_OK, pdu->caMsg, pdu->uiMsgLen))
		{
			printf("注册成功，获得ID为：%u\n", pdu->uiTo);
			g_uiId = pdu->uiTo;
		}
	}	
	else
	{
		printf("注册失败...\n");
	}
}

int login(int sockfd)
{
	unsigned int uiId;
	char caPwd[32] = {'\0'};
	printf("请输入登入ID:\n");
	scanf("%d", &uiId);
	printf("请输入登入密码:\n");
	getStrFromSTDIN(caPwd, 32);

	//生成登入请求信息发送给服务器
	PDU *pdu = mkPDU(strlen(caPwd)+1);
	pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
	pdu->uiFrom = uiId;
	strcpy(pdu->caMsg,caPwd);
	sendPDU(sockfd, pdu);
	free(pdu);
	pdu = NULL;

	//接收服务器的登入回复信息
	pdu = recvPDU(sockfd);
	int ret = 0;
	if (ENUM_MSG_TYPE_LOGIN_RESPOND == pdu->uiMsgType)
	{
		if (0 == strcmp(LOGIN_OK, pdu->caMsg))
		{
			g_uiId = uiId;
			ret = 1;
		}
		printf("%s\n", pdu->caMsg);
	}
	return ret;
}

int loginOrRegistFace()
{
	printf("  ^_^\n");
	printf("1,登录\n");
	printf("2,注册\n");
	printf("0,返回\n");
	printf("请输入操作选项:\n");
	int num = 0;
	scanf("%d", &num);
	return num;
}

void loginOrRegist(int sockfd)
{
	int num = 0;
	int ret = 0;
	while (1)
	{
		num = loginOrRegistFace();
		switch (num)
		{
		case 2: 
			regist(sockfd);  //测试注册
			break;
		case 1:
			ret = login(sockfd);     //测试登入
			break;
		case 0:
			close(sockfd);   //和服务器断开连接
			exit(EXIT_SUCCESS);
		default:
			printf("输入有误，请重新输入\n");
			break;
		}
		if (1 == ret)
		{
			break;
		}
	}
}

int chatFace()
{
	printf("  ^_^\n");
	printf("1,获得好友列表\n");
	printf("2,私聊\n");
	printf("3,群聊\n");
	printf("0,退出登入\n");
	printf("请输入操作选项:\n");
	int num = 0;
	scanf("%d", &num);
	return num;
}
			
void handleGetFriendRespond(PDU *pdu)
{
	if (NULL == pdu)
	{
		return;
	}
	printf("获得的好友列表为:\n");
	unsigned int uiId = 0;
	int num =  (pdu->uiMsgLen)/sizeof(unsigned int);
	int i = 0;
	printf("num = %d\n", num);
	for (; i < num; i++)
	{
		memcpy(&uiId, pdu->caMsg+i*sizeof(unsigned int)
			   , sizeof(unsigned int));
		printf("\t好友ID: %d\n", uiId);
	}
}

void *handleServer(int sockfd)
{
	PDU *pdu = NULL;
	while (1)
	{
		pdu = recvPDU(sockfd);
		if (NULL == pdu)
		{
			printf("和服务器断开了连接\n");
			exit(EXIT_FAILURE);
		}
		switch (pdu->uiMsgType)
		{
		case ENUM_MSG_TYPE_GET_FRIEND_RESPOND:
			handleGetFriendRespond(pdu);
			break;
		case ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND:
			printf("信息发送失败:%s\n", pdu->caMsg);
			break;
		case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
	//		printf("%d说:%s\n", pdu->uiFrom, pdu->caMsg);
	//		break;
		case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
			printf("%d说:%s\n", pdu->uiFrom, pdu->caMsg);
			break;
		default:
			break;			
		}
	}
}
			
void getFriendList(int sockfd)
{
	PDU *pdu = mkPDU(0);
	pdu->uiMsgType = ENUM_MSG_TYPE_GET_FRIEND_REQUEST;
	sendPDU(sockfd, pdu);
	free(pdu);
	pdu = NULL;
}
			
void privateChat(int sockfd)
{
	unsigned int uiId = 0;
	char caMsg[4096] = {'\0'};
	printf("请输入对方的ID:\n");
	scanf("%d", &uiId);
	printf("请输入聊天信息:\n");
	getStrFromSTDIN(caMsg, 4096);

	PDU *pdu = mkPDU(strlen(caMsg)+1);
	pdu->uiTo = uiId;
	pdu->uiFrom = g_uiId;
	strcpy(pdu->caMsg, caMsg);
	pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
	sendPDU(sockfd, pdu);
	free(pdu);
	pdu = NULL;
}

void groupChat(int sockfd)
{
	char caMsg[4096] = {'\0'};
	printf("请输入群聊信息:\n");
	getStrFromSTDIN(caMsg, 4096);
	PDU *pdu = mkPDU(strlen(caMsg)+1);
	strcpy(pdu->caMsg, caMsg);
	pdu->uiFrom = g_uiId;
	pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
	sendPDU(sockfd, pdu);
	free(pdu);
	pdu = NULL;
}

void chat(int sockfd)
{
	int num = 0;
	while (1)
	{
		num = chatFace();
		switch (num)
		{
		case 1:
			getFriendList(sockfd);
			break;
		case 2:
			privateChat(sockfd);
			break;
		case 3:
			groupChat(sockfd);
			break;
		case 0:
			close(sockfd);
			exit(EXIT_SUCCESS);
			break;
		default:
			printf("输入有误，请重新输入\n");
			break;
		}
	}
}


int main(void)
{
	int sockfd = mkSocket(); //产生sockfd
	connectToServer(sockfd); //通过sockfd连接服务器
	loginOrRegist(sockfd);   //登入注册
	pthread_t thr;
	//创建线程专门处理服务器发送过来的信息
	pthread_create(&thr, NULL, handleServer, (void*)sockfd);
	chat(sockfd);            //聊天
	
	return 0;
}
