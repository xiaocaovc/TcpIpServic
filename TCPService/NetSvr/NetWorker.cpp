#include "stdafx.h"
#include "NetWorker.h"
#define EXIT_CODE NULL
#define MAX_CONNECT 1024

CWorker::CWorker()
	:m_hIocp(NULL)
	,m_handle(NULL)
	, m_nConnect(0)
{
}


CWorker::~CWorker()
{
}


// 停止
bool CWorker::Stop()
{
	if (!m_hIocp)
	{
		return true;
	}
	if (m_handle)
	{
		PostQueuedCompletionStatus(m_hIocp, 0, (DWORD)EXIT_CODE, NULL);
		WaitThreadClose(m_handle);
	}
	return true;
}


int CWorker::Init()
{
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_hIocp)
	{
		TRACE(_T("建立完成端口失败！错误代码: %d！"), WSAGetLastError());
		return -1;
	}
	DWORD nThreadID = 0;
	m_handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)WorkerThread, (void *)this, 0, &nThreadID);
	if (!m_handle)
	{
		return -1;
	}
	m_Overlapped.lpData = new char[BUF_LEN];
	return 0;
}

// 工作线程
DWORD CWorker::WorkerThread(CWorker* lpVoid)
{
	OVERLAPPEDEX* lpOverlapped = NULL;
	COMPLETIONKEYEX* lpCompletionKey = NULL;
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
			if (!lpOverlapped)
			{
				lpVoid->PostRecv(lpCompletionKey, &lpVoid->m_Overlapped);
				continue;
			}
			if (!bReturn || dwBytesTransfered == 0)
			{
				switch (lpOverlapped->m_OpType)
				{
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
				switch (lpOverlapped->m_OpType)
				{
				case SEND_POSTED:
				{
				}
				break;
				case RECV_POSTED:
				{
					lpVoid->PostRecv(lpCompletionKey, &lpVoid->m_Overlapped);
				}
				default:
					break;
				}
			}
		}
	}
	return 0;
}


// 获得IOCP句柄
HANDLE CWorker::GetIocp()
{
	if (MAX_CONNECT>m_nConnect)
	{
		m_nConnect++;
		return m_hIocp;
	}
}
