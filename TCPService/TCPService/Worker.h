#pragma once
#include <winsock2.h>
#include <MSWSock.h>
#include"Poolx.h"

typedef enum net_operation_s
{
	NULL_POSTED,    // 用于初始化，无意义
	ACCEPT_POSTED, // 标志投递的Accept操作
	SEND_POSTED,   // 标志投递的是发送操作
	RECV_POSTED,   // 标志投递的是接收操作
	WRITE_POSTED,  // 标志投递的是写入操作
	READ_POSTED,   // 标志投递的是读取操作

}net_operation_t;
typedef struct net_completionkeyex_s net_completionkeyex_t;
typedef struct net_overlappedex_s net_overlappedex_t;
struct net_completionkeyex_s
{
	OVERLAPPED      overlapped; // 重叠结构
	net_operation_t  optype;     // 操作标识
	SOCKET s;
	char buffer[(sizeof(SOCKADDR_IN) + 16) * 2];
	DWORD dwbytes;
	DWORD dwflags;
	net_completionkeyex_t*   next; // 0,1
};
typedef struct net_overlappedex_s
{
	OVERLAPPED      overlapped; // 重叠结构
	net_operation_t  optype;     // 操作标识
	WSABUF			wsabuf[1];
};
class CWorker
{
public:
	CWorker();
	~CWorker();
	//等待停止线程
	static void WaitThreadClose(HANDLE handle)
	{
		MSG msg;
		DWORD result;
		while (handle != NULL)
		{
			result = MsgWaitForMultipleObjects(1, &handle, false, INFINITE, QS_ALLINPUT);//INFINITE
			switch (result)
			{
			case WAIT_OBJECT_0: //线程的结束
				CloseHandle(handle);
				handle = NULL;
				break; //break the loop
			case WAIT_OBJECT_0 + 1:
				////主线程里使用GetSafeHwnd(),辅佐线程用GetForegroundWindow()获得窗口句柄
				PeekMessage(&msg, GetForegroundWindow(), 0, 0, PM_REMOVE);
				DispatchMessage(&msg);
				continue;
			default:
				return;/// unexpected failure 
			}
		}
	}
	bool PostRecv(net_completionkeyex_t* lpCompletionKey, int size)
	{
		DWORD dwbytes = 0;
		DWORD dwflags = 0;
		net_overlappedex_t* lpOverlapped = (net_overlappedex_t*)plx_palloc(pool,size + sizeof(net_overlappedex_t));
		lpOverlapped->optype = RECV_POSTED;
		// 初始化
		lpOverlapped->wsabuf[0].buf = (char*)((char*)lpOverlapped + sizeof(net_overlappedex_t));
		lpOverlapped->wsabuf[0].len = size;
		// 投递WSARecv请求
		ZeroMemory(&lpOverlapped->overlapped, sizeof(OVERLAPPED));
		int nRes = WSARecv(lpCompletionKey->s, &lpOverlapped->wsabuf[0], 1, &dwbytes,
			&dwflags, &lpOverlapped->overlapped, NULL);
		if (SOCKET_ERROR == nRes)
		{
			nRes = WSAGetLastError();
			if (WSA_IO_PENDING != nRes)
			{
				TRACE("---服务端接收错误SOCKET:%d,释放资源,错误代码:%d---\n", lpCompletionKey->s, nRes);
				return false;
			}
			return true;
		}
		return true;
	}
private:
	// 工作线程句柄
	HANDLE m_handle;
	HANDLE m_hIocp;
	// 工作线程
	static DWORD WorkerThread(CWorker* lpVoid);
	plx_pool_t* pool;
public:
	int Run();
	HANDLE GetHocp();
	int Stop();
	int m_active;
};

