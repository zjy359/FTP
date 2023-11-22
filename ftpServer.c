#include <stdio.h>
#include"ftpServer.h"
char g_recvBuf[10240];
int g_fileSize;
char* g_fileBuf;
#pragma warning(disable : 4996)

int main()
{
	initSocket();
	listenToClient();
	closeSocket();
	return 0;
}




bool initSocket()
{
	WSADATA wsadata;
	
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		printf("WSAStartup faild :%d\n", WSAGetLastError());
		return FALSE;
	}
	return TRUE;

}

bool closeSocket()
{
	if (0 != WSACleanup())
	{
		printf("WSACleaup faild:%d\n", WSAGetLastError());
		return FALSE;
	}
	return TRUE;
}

void listenToClient()
{
	printf("listen to client....\n");

	SOCKET serfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serfd == INVALID_SOCKET)
	{
		err("socket");
		return;
	}

	struct sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(SPORT);
	serAddr.sin_addr.S_un.S_addr = ADDR_ANY;
	
	if (0 != bind(serfd, (struct sockaddr*)&serAddr, sizeof(serAddr)))
	{
		err("bind");
		return;
	}
	
	if (0 != listen(serfd, 10))
	{
		err("listen");
		return;
	}
	
	struct sockaddr_in cliAddr;
	int len = sizeof(cliAddr);
	SOCKET clifd = accept(serfd, (struct sockaddr*)&cliAddr, &len);
	if (INVALID_SOCKET == clifd)
	{
		err("accept");
		return;
	}
	printf("new client connect success....\n");

	while (processMsg(clifd)) {}
}

bool processMsg(SOCKET clifd)
{
	int nRes = recv(clifd, g_recvBuf, 10240, 0);
	
	if (nRes <= 0)
	{
		printf("recv error%d\n", WSAGetLastError());
		return false;
	}
	
	struct MsgHeader* msg = (struct MsgHeader*)g_recvBuf;
	
	struct MsgHeader exitmsg;
	switch (msg->msgID)
	{
	case MSG_FILENAME:
		printf("文件名：%s\n", msg->fileInfo.fileName);
		readFile(clifd, msg);
		break;
	case MSG_SENDFILE:
		sendFile(clifd, msg);
		break;
	case MSG_SUCCESSED:
		exitmsg.msgID = MSG_SUCCESSED;
		if (SOCKET_ERROR == send(clifd, (char*)&exitmsg, sizeof(struct MsgHeader), 0))
		{
			printf("send faild :%d\n", WSAGetLastError());
			return false;
		}
		printf("发送成功~\n");
		break;
	case ENDSEND:
		printf("发送完成 bye~bye~\n");
		closesocket(clifd);
		return FALSE;
		break;
	}
	
	return true;
}


bool readFile(SOCKET clifd, struct MsgHeader* pmsg)
{
	FILE* pread = fopen(pmsg->fileInfo.fileName, "rb");
	
	if (pread == NULL)
	{
		printf("[%s]\n", pmsg->fileInfo.fileName);
		struct MsgHeader msg;
		msg.msgID = MSG_OPENFILE_FAILD;
		if (SOCKET_ERROR == send(clifd, (char*)&msg, sizeof(struct MsgHeader), 0))
		{
			printf("send faild:%d\n", WSAGetLastError());
		}
		return FALSE;
	}
	
	fseek(pread, 0, SEEK_END);
	g_fileSize = ftell(pread);
	fseek(pread, 0, SEEK_SET);
	struct MsgHeader msg;
	msg.msgID = MSG_FILESIZE;
	
	msg.fileInfo.fileSize = g_fileSize;
	char tfname[200] = { 0 }, text[100];
	_splitpath(pmsg->fileInfo.fileName, NULL, NULL, tfname, text);
	strcat(tfname, text);
	
	strcpy(msg.fileInfo.fileName, tfname);
	send(clifd, (char*)&msg, sizeof(struct MsgHeader), 0);

	g_fileBuf = calloc(g_fileSize + 1, sizeof(char));
	if (g_fileBuf == NULL)
	{
		printf("内存不足\n");
		return false;
	}
	fread(g_fileBuf, sizeof(char), g_fileSize, pread);
	g_fileBuf[g_fileSize] = '\0';
	fclose(pread);
	return true;
}

bool sendFile(SOCKET clifd, struct MsgHeader* pmsg)
{
	
	struct MsgHeader msg;
	msg.msgID = MSG_READY_READ;

	for (size_t i = 0; i < g_fileSize; i += PACKET_SIZE)
	{
		msg.packet.nStart = i;
		
		if (i + PACKET_SIZE + 1 > g_fileSize)
		{
			msg.packet.nsize = g_fileSize - i;
		}
		else
		{
			msg.packet.nsize = PACKET_SIZE;
		}
		memcpy(msg.packet.buf, g_fileBuf + msg.packet.nStart, msg.packet.nsize);
		if (SOCKET_ERROR == send(clifd, (char*)&msg, sizeof(struct MsgHeader), 0))
		{
			printf("send error %d\n", WSAGetLastError());
			return false;
		}
	}

	return TRUE;
}