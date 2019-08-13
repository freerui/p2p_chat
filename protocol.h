#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef unsigned int uint;

#define REGIST_OK       "regist ok"
#define REGIST_FAILURE  "regist failure"

#define NO_USR     "此用户不存在"
#define PWD_ERROR  "密码错误"
#define LOGIN_OK   "登录成功"
#define OFFLINE    "对方不在线"

enum ENUM_MSG_TYPE
{
	ENUM_MSG_TYPE_MIN = 0,
	
	ENUM_MSG_TYPE_REGIST_REQUEST,       //注册请求
	ENUM_MSG_TYPE_REGIST_RESPOND,       //注册回复
	
	ENUM_MSG_TYPE_LOGIN_REQUEST,        //登入请求
	ENUM_MSG_TYPE_LOGIN_RESPOND,        //登入回复
	
	ENUM_MSG_TYPE_GET_FRIEND_REQUEST,   //获得好友请求
	ENUM_MSG_TYPE_GET_FRIEND_RESPOND,   //获得好友回复
	
	ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST, //私聊请求
	ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND, //私聊回复
	
	ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,   //群聊请求
	ENUM_MSG_TYPE_GROUP_CHAT_RESPOND,   //群聊回复

	ENUM_MSG_TYPE_MAX = 0x00ffffff
};

typedef struct PDU  //协议数据单元
{
	uint uiPDULen;     //整个发送数据的大小
	uint uiMsgType;    //消息类型
	uint uiFrom;       //谁发送的
	uint uiTo;         //发送给谁
	uint uiMsgLen;     //实际消息的长度
	char caMsg[4];     //指向实际消息
}PDU;

PDU *mkPDU(uint uiMsgLen);
void sendPDU(int sockfd, const PDU *pdu);
PDU *recvPDU(int sockfd);

#endif
