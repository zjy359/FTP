#pragma once
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#include<stdbool.h>
#define SPORT 8888
#define err(errMsg) printf("[line:%d]%s failed code %d" ,__LINE__,errMsg, WSAGetLastError());

enum MSGTAG
{
	MSG_FILENAME = 1,
	MSG_FILESIZE = 2,
	MSG_READY_READ = 3,
	MSG_SENDFILE = 4,
	MSG_SUCCESSED = 5,
	MSG_OPENFILE_FAILD = 6,
	ENDSEND = 7
};


#pragma pack(1)
#define PACKET_SIZE (10240 -sizeof(int)*3)
struct MsgHeader
{
	enum MSGTAG msgID;
	union MyUnion
	{
		struct
		{
			char fileName[256];
			int fileSize;
		}fileInfo; //260
		struct
		{
			int nStart;
			int nsize;
			char buf[PACKET_SIZE];
		}packet;
	};
};
#pragma pack()



bool initSocket();

bool closeSocket();

void listenToClient();

bool processMsg(SOCKET);

bool readFile(SOCKET, struct MsgHeader*);

bool sendFile(SOCKET, struct MsgHeader*);

