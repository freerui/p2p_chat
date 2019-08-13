#include "protocol.h"
#include <unistd.h>

PDU *mkPDU(uint uiMsgLen)
{
	uint uiPDULen = sizeof(PDU)-4*sizeof(char)+uiMsgLen;
	PDU *pdu = (PDU*)malloc(uiPDULen);
	if (NULL == pdu)
	{
		printf("malloc pdu failed\n");
		exit(EXIT_FAILURE);
	}
	memset(pdu, 0, uiPDULen);
	pdu->uiPDULen = uiPDULen;
	pdu->uiMsgLen = uiMsgLen;

	return pdu;
}

void sendPDU(int sockfd, const PDU *pdu)
{
	if (NULL == pdu)
	{
		return;
	}
	uint uiLeft = pdu->uiPDULen;
	uint uiSended = 0;
	ssize_t ret = -1;
	while (1)
	{
		if (uiLeft > 4096)
		{
			ret = write(sockfd, (char*)pdu+uiSended, 4096);
		}
		else
		{
			ret = write(sockfd, (char*)pdu+uiSended, uiLeft);
		}
		if (-1 == ret || 0 == ret)
		{
			break;
		}
		uiLeft -= ret;
		uiSended += ret;
	}
}

PDU *recvPDU(int sockfd)
{
	uint uiPDULen = 0;
	ssize_t ret = -1;
	ret = read(sockfd, &uiPDULen, sizeof(uint));
	if (0 == ret || -1 == ret)
	{
		return NULL;
	}
	uint uiMsgLen = uiPDULen-(sizeof(PDU)-4*sizeof(char));	
	PDU *pdu = mkPDU(uiMsgLen);
	
	uint uiLeft = uiPDULen-sizeof(uint);
	uint uiRecved = sizeof(uint);
	while (1)
	{
		if (uiLeft > 4096)
		{
			ret = read(sockfd, (char*)pdu+uiRecved, 4096);
		}
		else
		{
			ret = read(sockfd, (char*)pdu+uiRecved, uiLeft);
		}
		if (-1 == ret || 0 == ret)
		{
			break;
		}
		uiLeft -= ret;
		uiRecved += ret;
		if (0 == uiLeft)
		{
			break;
		}
	}

	return pdu;
}
