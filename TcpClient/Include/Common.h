#ifndef _COMMON_STRUCT_IO_
#define _COMMON_STRUCT_IO_

typedef struct worker_data_s worker_data_t;
typedef struct connect_key_s connect_key_t;
typedef struct overlapped_s overlapped_t;
typedef void(*BackFun)(void* lpKey, void* lpdata, int optype, int len, void* lpobj);
typedef enum net_optype_s
{
	NULL_POSTED,    // 用于初始化，无意义
	ACCEPT_POSTED, // 标志投递的Accept操作
	CONNECT_POSTED,
	SEND_POSTED,   // 标志投递的是发送操作
	RECV_POSTED,   // 标志投递的是接收操作
	WRITE_POSTED,  // 标志投递的是写入操作
	READ_POSTED,   // 标志投递的是读取操作

}net_optype_t;
struct worker_data_s
{
	HANDLE hIocp;
	int connect;
};
struct connect_key_s
{
	OVERLAPPED      overlapped; // 重叠结构
	net_optype_t    optype;
	char ip[20];
	unsigned port;
	SOCKET s;
	HANDLE hIocp; // 处理连接的IOCP队列(线程)
	BackFun func;
	void* obj;
	connect_key_t* next;
};
struct overlapped_s
{
	OVERLAPPED      overlapped; // 重叠结构
	net_optype_t    optype;
	WSABUF			wsabuf;
};

#endif