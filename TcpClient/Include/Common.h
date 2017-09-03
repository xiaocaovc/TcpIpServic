#ifndef _COMMON_STRUCT_IO_
#define _COMMON_STRUCT_IO_

typedef struct worker_data_s worker_data_t;
typedef struct connect_key_s connect_key_t;
typedef struct overlapped_s overlapped_t;
typedef void(*BackFun)(void* lpKey, void* lpdata, int optype, int len, void* lpobj);
typedef enum net_optype_s
{
	NULL_POSTED,    // ���ڳ�ʼ����������
	ACCEPT_POSTED, // ��־Ͷ�ݵ�Accept����
	CONNECT_POSTED,
	SEND_POSTED,   // ��־Ͷ�ݵ��Ƿ��Ͳ���
	RECV_POSTED,   // ��־Ͷ�ݵ��ǽ��ղ���
	WRITE_POSTED,  // ��־Ͷ�ݵ���д�����
	READ_POSTED,   // ��־Ͷ�ݵ��Ƕ�ȡ����

}net_optype_t;
struct worker_data_s
{
	HANDLE hIocp;
	int connect;
};
struct connect_key_s
{
	OVERLAPPED      overlapped; // �ص��ṹ
	net_optype_t    optype;
	char ip[20];
	unsigned port;
	SOCKET s;
	HANDLE hIocp; // �������ӵ�IOCP����(�߳�)
	BackFun func;
	void* obj;
	connect_key_t* next;
};
struct overlapped_s
{
	OVERLAPPED      overlapped; // �ص��ṹ
	net_optype_t    optype;
	WSABUF			wsabuf;
};

#endif