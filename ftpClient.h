#pragma once
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#include<stdbool.h>
#define SPORT 8888
#define err(errMsg) printf("[line:%d]%s failed code %d" ,__LINE__,errMsg, WSAGetLastError());

//����־λ
enum MSGTAG
{
	MSG_FILENAME = 1,
	MSG_FILESIZE = 2,
	MSG_READY_READ = 3,
	MSG_SENDFILE = 4,
	MSG_SUCCESSED = 5,
	MSG_OPENFILE_FAILD = 6,
	ENDSEND = 7,
	//CHAT_REQUSET = 8,
	//CHAT_START = 9,
	CHAT_END = 10,
	CHAT_ING = 11
};

//���ṹ
#pragma pack(1)
#define PACKET_SIZE (102400-sizeof(int)*3)//!!!!
struct MsgHeader
{
	enum MSGTAG msgID;//��ǰ��Ϣ���
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
			int nsize;//�ð������ݴ�С
			char buf[PACKET_SIZE];
		}packet;
	};
};
#pragma pack()


//��ʼ��socket��
bool initSocket();
//�ر�socket��
bool closeSocket();
//�����ͻ�������
void connectToHost();
//������Ϣ
bool processMsg(SOCKET);
//��ȡ�ļ���
void downloadFileName(SOCKET serfd);
//���ݷ��ص��ļ���С�����ڴ�ռ�
void readyread(SOCKET serfd, struct MsgHeader*);
//д�ļ�
bool writeFile(SOCKET serfd, struct MsgHeader*);
//������
bool chatroom(SOCKET);
//���̷߳��ͺ���
unsigned __stdcall p_send(void*);
//���߳̽��ܺ���
unsigned __stdcall p_recv(void*);