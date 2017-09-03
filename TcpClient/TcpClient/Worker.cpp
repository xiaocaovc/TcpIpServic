#include "stdafx.h"
#include "Worker.h"


CWorker::CWorker(HANDLE hIocp)
	:m_hIocp(NULL)
	, m_phIocp(hIocp)
	, pool(NULL)
{
}


CWorker::~CWorker()
{
}

int CWorker::Stop()
{
	if (m_hIocp)
	{
		PostQueuedCompletionStatus(m_hIocp, 0, (DWORD)EXIT_CODE, NULL);
		if (m_handle)
		{
			WaitThreadClose(m_handle);
		}
		CloseHandle(m_hIocp);
	}
	return 0;
}
int CWorker::PostRecv(connect_key_t* lpCompletionKey, int size)
{
	DWORD dwbytes = 0;
	DWORD dwflags = 0;
	overlapped_t* lpOverlapped = (overlapped_t*)plx_palloc(pool, size + sizeof(overlapped_t));
	lpOverlapped->optype = RECV_POSTED;
	// 初始化
	lpOverlapped->wsabuf.buf = (char*)((char*)lpOverlapped + sizeof(overlapped_t));
	lpOverlapped->wsabuf.len = size;
	// 投递WSARecv请求
	ZeroMemory(&lpOverlapped->overlapped, sizeof(OVERLAPPED));
	int nRes = WSARecv(lpCompletionKey->s, &lpOverlapped->wsabuf, 1, &dwbytes,
		&dwflags, &lpOverlapped->overlapped, NULL);
	if (SOCKET_ERROR == nRes)
	{
		nRes = WSAGetLastError();
		if (WSA_IO_PENDING != nRes)
		{
			TRACE("---服务端接收错误SOCKET:%d,释放资源,错误代码:%d---\n", lpCompletionKey->s, nRes);
			return -1;
		}
	}
	return 0;
}
int CWorker::PostSend(connect_key_t* lpCompletionKey, overlapped_t* lpOverlapped)
{
	lpOverlapped->optype = SEND_POSTED;
	DWORD dwbytes = 0;
	DWORD dwflags = 0;
	ZeroMemory(&lpOverlapped->overlapped, sizeof(OVERLAPPED));
	int nRes = WSASend(lpCompletionKey->s, &lpOverlapped->wsabuf, 1, &dwbytes,
		dwflags, &lpOverlapped->overlapped, NULL);
	if (SOCKET_ERROR == nRes)
	{
		nRes = WSAGetLastError();
		if (WSA_IO_PENDING != nRes)
		{
			TRACE("---_COMPLETION_KEY_NET::PostSend 服务端发送失败SOCKET:%d,释放资源,错误代码:%d!---\n", m_Handle.m_Socket, nRes);
			return -1;
		}
	}
	return 0;
}
HANDLE CWorker::Init()
{
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)0, 0);
	DWORD nThreadID = 0;
	m_handle = ::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)WorkerThread, (void *)this, 0, &nThreadID);
	if (!m_handle)
	{
		return NULL;
	}
	pool = plx_create_pool(POOL_SIZE);
	return m_hIocp;
}
// 工作线程
DWORD CWorker::WorkerThread(CWorker* lpVoid)
{
	void* lpOverlapped = NULL;
	void* lpCompletionKey = NULL;
	DWORD dwBytesTransfered = 0;
	BOOL bReturn = FALSE;
	while (true)
	{
		bReturn = GetQueuedCompletionStatus(lpVoid->m_hIocp,
			&dwBytesTransfered, (PULONG_PTR)&lpCompletionKey, (LPOVERLAPPED *)&lpOverlapped,
			INFINITE);
		// 如果收到的是退出标志，则直接退出
		if (EXIT_CODE == (DWORD)lpCompletionKey)
		{
			break;
		}
		else
		{
		}
	}
	return 0;
}

