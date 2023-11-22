#include <stdio.h>
#include"ftpClient.h"
#include"string.h"
#pragma warning(disable : 4996)
char g_recvBuf[10240];      
char* g_fileBuf;       
int g_fileSize;         
char g_fileName[256];        

int main()
{
	initSocket();
	connectToHost();
	closeSocket();
	return 0;
}

bool initSocket()
{
	WSADATA wsadata;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		err("WSAStartup");
		return FALSE;
	}
	return TRUE;

}

bool closeSocket()
{
	if (0 != WSACleanup())
	{
		err("WSACleaup");
		return FALSE;
	}
	return TRUE;

}

void connectToHost()
{
	
	//INVALID_SOKET SOCKET_ERROR
	SOCKET  serfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serfd == INVALID_SOCKET)
	{
		printf("socket faild :%d\n", WSAGetLastError());
		return;
	}
	
	struct sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(SPORT);
	serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	
	if (0 != connect(serfd, (struct sockaddr*)&serAddr, sizeof(serAddr)))
	{
		printf("connnect faild :%d\n", WSAGetLastError());
		return;
	}
	printf("conect success!");

	downloadFileName(serfd);
	while (processMsg(serfd)) {
		Sleep(100);
	};

	return true;
}


bool processMsg(SOCKET serfd)
{
	recv(serfd, g_recvBuf, 10240, 0);
	struct MsgHeader* msg = (struct MsgHeader*)g_recvBuf;
	switch (msg->msgID)
	{
	case MSG_OPENFILE_FAILD:
		printf("send file failed ,please try again\n");
		downloadFileName(serfd);
		break;
	case MSG_FILESIZE:
		readyread(serfd, msg);
		break;
	case MSG_READY_READ:
		writeFile(serfd, msg);
		break;
	case MSG_SUCCESSED:
		printf("send file success!\n");

		struct MsgHeader file;
		file.msgID = ENDSEND;
		if (SOCKET_ERROR == send(serfd, (char*)&file, sizeof(struct MsgHeader), 0))
		{
			err("send");
			return;
		}
		printf("文件发送完成\n");
		closesocket(serfd);
		break;
	}
	return TRUE;
}

void downloadFileName(SOCKET serfd)
{
	char fileName[1024];
	printf("请输入文件名");
	gets_s(fileName, 1024);
	struct MsgHeader file;
	file.msgID = MSG_FILENAME;
	strcpy(file.fileInfo.fileName, fileName);
	if (SOCKET_ERROR == send(serfd, (char*)&file, sizeof(struct MsgHeader), 0))
	{
		err("send");
		return;
	}
}

void readyread(SOCKET serfd, struct MsgHeader* pmsg)
{
	strcpy(g_fileName, pmsg->fileInfo.fileName);
	
	g_fileSize = pmsg->fileInfo.fileSize;

	g_fileBuf = calloc(g_fileSize + 1, sizeof(char));
	if (g_fileBuf == NULL)
	{
		printf("内存不足\n");
	}
	else
	{
		struct MsgHeader msg;
		msg.msgID = MSG_SENDFILE;
		if (SOCKET_ERROR == send(serfd, (char*)&msg, sizeof(struct MsgHeader), 0))
		{
			err("send");
			return;
		}
	}
}

bool writeFile(SOCKET serfd, struct MsgHeader* pmsg)
{

	if (g_fileBuf == NULL)
	{
		return FALSE;
	}

	int nStart = pmsg->packet.nStart;

	int nsize = pmsg->packet.nsize;

	memcpy(g_fileBuf + nStart, pmsg->packet.buf, nsize);

	if (nStart + nsize >= g_fileSize)
	{
		FILE* pwrite = fopen(g_fileName, "wb");
		if (pwrite == NULL)
		{
			printf("write file error..\n");
			return false;
		}
		fwrite(g_fileBuf, sizeof(char), g_fileSize, pwrite);
	
		fclose(pwrite);
		free(g_fileBuf);
		g_fileBuf = NULL;
		
		struct MsgHeader msg;
		msg.msgID = MSG_SUCCESSED;
		if (SOCKET_ERROR == send(serfd, (char*)&msg, sizeof(struct MsgHeader), 0))
		{
			err("send");
			return;
		}
	}
	return true;
}
