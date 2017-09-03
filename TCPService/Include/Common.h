#ifndef _COMMON_STRUCT_IO_
#define _COMMON_STRUCT_IO_

typedef enum net_operation_s
{
	NULL_POSTED,    // 用于初始化，无意义
	ACCEPT_POSTED, // 标志投递的Accept操作
	SEND_POSTED,   // 标志投递的是发送操作
	RECV_POSTED,   // 标志投递的是接收操作
	WRITE_POSTED,  // 标志投递的是写入操作
	READ_POSTED,   // 标志投递的是读取操作

}net_operation_t;
typedef struct net_completionkeyex_s
{
	SOCKET s;
	int   active; // 0,1
}net_completionkeyex_t;
typedef struct _overlappedex_
{
	OVERLAPPED      m_Overlapped; // 重叠结构
	OPERATION_TYPE  m_OpType;     // 操作标识
	DWORD			m_dwFlags;
	DWORD			m_dwBytes;
	WSABUF			m_wsaBuf[1];
}
#endif