#pragma once
#include <winsock2.h>
#define BUF_LEN 1024*2
typedef enum _OPERATION_TYPE
{
	NULL_POSTED,    // 用于初始化，无意义
	ACCEPT_POSTED, // 标志投递的Accept操作
	SEND_POSTED,   // 标志投递的是发送操作
	RECV_POSTED,   // 标志投递的是接收操作
	WRITE_POSTED,  // 标志投递的是写入操作
	READ_POSTED,   // 标志投递的是读取操作

}OPERATION_TYPE;
typedef struct _COMPLETIONKEYEX
{
	SOCKET socket;
	bool bBusy;
	_COMPLETIONKEYEX* next;
}COMPLETIONKEYEX;
typedef struct _COMPLETIONKEYEX_POOL
{
	SOCKET socket;
	bool bBusy;
	_COMPLETIONKEYEX* next;
}COMPLETIONKEYEX_POOL;
typedef struct _OVERLAPPEDEX
{
	OVERLAPPED      m_Overlapped; // 重叠结构
	OPERATION_TYPE  m_OpType;     // 操作标识
	SOCKET socket;
	char*  lpData;
	DWORD			m_dwFlags;
	DWORD			m_dwBytes;
	WSABUF			m_wsaBuf[1];
}OVERLAPPEDEX;
class CWorker
{
public:
	CWorker();
	~CWorker();
private:
	//等待停止线程
	void WaitThreadClose(HANDLE handle)
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
	bool PostRecv(COMPLETIONKEYEX* lpCompletionKey, OVERLAPPEDEX* lpOverlapped)
	{
		lpOverlapped->m_OpType = RECV_POSTED;
		lpOverlapped->m_dwBytes = 0;
		lpOverlapped->m_dwFlags = 0;
		// 初始化
		lpOverlapped->m_Overlapped.hEvent = NULL;
		lpOverlapped->m_wsaBuf[0].buf = lpOverlapped->lpData;
		lpOverlapped->m_wsaBuf[0].len = 10;
		// 投递WSARecv请求
		//ASSERT(INVALID_SOCKET!=m_Handle.m_Socket);
		ZeroMemory(&lpOverlapped->m_Overlapped, sizeof(lpOverlapped->m_Overlapped));
		int nRes = WSARecv(lpCompletionKey->socket, &lpOverlapped->m_wsaBuf[0], 1, &lpOverlapped->m_dwBytes,
			&lpOverlapped->m_dwFlags, &lpOverlapped->m_Overlapped, NULL);
		if (SOCKET_ERROR == nRes)
		{
			nRes = WSAGetLastError();
			if (WSA_IO_PENDING != nRes)
			{
				TRACE("---服务端接收错误SOCKET:%d,释放资源,错误代码:%d---\n", lpOverlapped->socket, nRes);
				return false;
			}
			return true;
		}
		return true;
	}
private:
	// 工作线程
	static DWORD /*WINAPI*/ WorkerThread(CWorker* lpVoid);
	// 完成端口,监听Recv Send
	HANDLE m_hIocp;
	// 监视线程
	HANDLE m_handle;
	OVERLAPPEDEX m_Overlapped;
public:
	// 停止
	bool Stop();
	int Init();
	// 获得IOCP句柄
	HANDLE GetIocp();
	// 连接数
	int m_nConnect;
};

