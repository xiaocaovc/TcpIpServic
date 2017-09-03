#include "stdafx.h"
#include "Worker.h"
#define EXIT_CODE NULL
#define MAX_ACTIVE_CONNECT 1024
extern LPFN_DISCONNECTEX gDisconnectEx;

CWorker::CWorker()
	:m_hIocp(NULL)
	, m_active(0)
	,pool(NULL)
{
	
}


CWorker::~CWorker()
{
	if (pool)
	{
		plx_destroy_pool(pool);
	}
}


int CWorker::Run()
{
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_hIocp)
	{
		TRACE(_T("建立完成端口失败！错误代码: %d！"), WSAGetLastError());
		return -1;
	}
	DWORD nThreadID = 0;
	m_handle = ::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)WorkerThread, (void *)this, 0, &nThreadID);
	if (!m_handle)
	{
		return -1;
	}
	pool=plx_create_pool(1024);
	return 0;
}


HANDLE CWorker::GetHocp()
{
	if (m_active > MAX_ACTIVE_CONNECT)
	{
		return NULL;
	}
	return m_hIocp;
}


int CWorker::Stop()
{

	return 0;
}
// 工作线程
DWORD CWorker::WorkerThread(CWorker* lpVoid)
{
	net_overlappedex_t* lpOverlapped = NULL;
	net_completionkeyex_t* lpCompletionKey = NULL;
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
			if (!bReturn || dwBytesTransfered == 0)
			{
				switch (lpOverlapped->optype)
				{
				case ACCEPT_POSTED:
				{
					/*ZeroMemory(&lpCompletionKey->overlapped, sizeof(lpCompletionKey->overlapped));
					gDisconnectEx(lpCompletionKey->s, &lpCompletionKey->overlapped, TF_REUSE_SOCKET, NULL);*/
					Sleep(1);
				}
				break;
				case SEND_POSTED:
				{
				}
				break;
				case RECV_POSTED:
				{
				}
				default:
					break;
				}
			}
			else // 接收或发送数据完成
			{
				switch (lpOverlapped->optype)
				{
				case ACCEPT_POSTED:
				{
					Sleep(1);
					lpVoid->PostRecv(lpCompletionKey, 1200);
					//ZeroMemory(&lpCompletionKey->overlapped, sizeof(lpCompletionKey->overlapped));
					//gDisconnectEx(lpCompletionKey->s, &lpCompletionKey->overlapped, TF_REUSE_SOCKET, NULL);
				}
				break;
				case SEND_POSTED:
				{
				}
				break;
				case RECV_POSTED:
				{
					plx_ralloc(lpVoid->pool, lpOverlapped);
					lpVoid->PostRecv(lpCompletionKey, 1200);
				}
				default:
					break;
				}
			}
		}
	}
	return 0;
}
