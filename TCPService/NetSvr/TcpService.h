#pragma once
#include <winsock2.h>
#include <MSWSock.h>
#include"NetWorker.h"
typedef struct _OVERLAPPEDE_ACCEPT
{
	OVERLAPPED      m_Overlapped; // 重叠结构
	OPERATION_TYPE  m_OpType;     // 操作标识
	SOCKET socket;
	char*  lpData;
	DWORD			m_dwFlags;
	DWORD			m_dwBytes;
	WSABUF			m_wsaBuf[1];
}OVERLAPPEDE_ACCEPT;

class CTcpService
{
public:
	CTcpService();
	~CTcpService();
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
private:

	// 获得连接句柄
	HANDLE GetConnectHandle();
	// 工作线程
	static DWORD /*WINAPI*/ WorkerThread(CTcpService* lpVoid);
	// 完成端口,监听Accept
	HANDLE m_hIocp;
	// 监视SOCKET
	SOCKET m_Socket;
	LPFN_ACCEPTEX m_lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS m_lpfnGetAcceptExSockAddrs;
	// 监视线程
	HANDLE m_handle;
	OVERLAPPEDE_ACCEPT   m_Overlapped;
	OVERLAPPEDE_ACCEPT m_Recv;
	CWorker* m_lpWork;
	// Worker数目
	int m_nWorker;
	bool PostRecv(OVERLAPPEDE_ACCEPT* lpOverlapped)
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
		int nRes = WSARecv(lpOverlapped->socket, &lpOverlapped->m_wsaBuf[0], 1, &lpOverlapped->m_dwBytes,
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
	bool PostAccept();
public:
	// 初始化服务器
	int Init(unsigned short nPort, char* szIP=NULL);
	bool Stop();
};

