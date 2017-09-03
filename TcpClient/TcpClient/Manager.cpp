#include "stdafx.h"
#include "Manager.h"
LPFN_CONNECTEX gConnectEx = NULL;
LPFN_DISCONNECTEX gDisconnectEx=NULL;

CManager::CManager()
	:m_handle(NULL)
	, m_hIocp(NULL)
	, m_actqueue(NULL)
{
}


CManager::~CManager()
{
	for (iter = workermap.begin(); iter != workermap.end(); iter++)
	{
		delete iter->first;
	}
	WSACleanup();
}

int CManager::Stop()
{
	if(m_hIocp)
	{
		PostQueuedCompletionStatus(m_hIocp, 0, (DWORD)EXIT_CODE, NULL);
		if (m_handle)
		{
			CWorker::WaitThreadClose(m_handle);
		}
	}
	CloseHandle(m_hIocp);
	for (iter = workermap.begin(); iter != workermap.end(); iter++)
	{
		iter->first->Stop();
	}
	return 0;
}

int CManager::GetFunAdr(SOCKET s)
{
	if (gConnectEx)
	{
		return 0;
	}
	DWORD dwBytes = 0;
	GUID GuidConnectEx = WSAID_CONNECTEX;
	// 重点，获得ConnectEx 函数的指针
	if (SOCKET_ERROR == WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,

		&GuidConnectEx, sizeof(GuidConnectEx),

		&gConnectEx, sizeof(LPFN_CONNECTEX), &dwBytes, 0, 0))

	{
		TRACE(_T("GuidConnectEx:WSAIoctl is failed. Error code = %d"), WSAGetLastError());
		return -1;
	}
	GUID GuidDisconnectEx = WSAID_DISCONNECTEX;
	if (SOCKET_ERROR == WSAIoctl(s,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidDisconnectEx,
		sizeof(GuidDisconnectEx),
		&gDisconnectEx,
		sizeof(LPFN_DISCONNECTEX),
		&dwBytes,
		NULL,
		NULL))
	{
		TRACE("GuidDisconnectEx:WSAIoctl is failed. Error code = %d !", WSAGetLastError());
		return -1;
	}
	return 0;
}

int CManager::Init()
{
	WSADATA wsaData;
	// 错误(一般都不可能出现)
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		TRACE("---初始化WinSock 2.2失败!---\n");
		return -1;
	}
	SOCKET s = ::WSASocket(AF_INET,SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	GetFunAdr(s);
	closesocket(s);
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)0, 0);
	DWORD nThreadID = 0;
	m_handle = ::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ManThread, (void *)this, 0, &nThreadID);
	if (!m_handle)
	{
		return -1;
	}
	pool = plx_create_pool(POOL_SIZE);
	// create work thread
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	int nWorker = si.dwNumberOfProcessors * 2;
	for (int i = 0; i < nWorker; i++)
	{
		CWorker* w = new CWorker(m_hIocp);
		if (w)
		{
			worker_data_t d;
			d.hIocp = w->Init();
			if (d.hIocp)
			{
				d.connect = 0;
				workermap[w] = d;
			}
			else
			{
				delete	w;
			}
		}
	}
	return 0;
}

DWORD CManager::ManThread(CManager* lpVoid)
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

int CManager::ConnectToServer(unsigned short nSvrPort, char* szSvrIP ,
	BackFun backfun, void* pObj)
{
	// 
	connect_key_t* cn = NULL;
	if (m_actqueue)
	{

	}
	else
	{
		cn = (connect_key_t*)plx_palloc(pool, sizeof(connect_key_t));
		cn->s= ::WSASocket(AF_INET,SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
		sockaddr_in local_addr;
		ZeroMemory(&local_addr, sizeof(sockaddr_in));
		local_addr.sin_family = AF_INET;
		int irt = ::bind(cn->s, (sockaddr *)(&local_addr), sizeof(sockaddr_in));
	}
	cn->hIocp = NULL;
	static std::map<CWorker*, worker_data_t>::iterator siter = workermap.begin();
	for (iter = siter; iter != workermap.end(); )
	{
		if (iter->second.connect < MAX_ACTIVE_CONNECT)
		{
			cn->hIocp = iter->second.hIocp;
			CreateIoCompletionPort((HANDLE)cn->s, cn->hIocp, (ULONG_PTR)cn, 0);
			return;
		}
		iter++;
		if (iter==siter)
		{
			break;
		}
	}
	// 设置连接目标地址
	if (!cn->hIocp)
	{
		return -1;
	}
	sockaddr_in addrPeer;
	ZeroMemory(&addrPeer, sizeof(sockaddr_in));
	addrPeer.sin_family = AF_INET;
	addrPeer.sin_addr.s_addr = inet_addr(szSvrIP);
	addrPeer.sin_port = htons(nSvrPort);
	int nLen = sizeof(addrPeer);
	PVOID lpSendBuffer = NULL;
	DWORD dwSendDataLength = 0;
	DWORD dwBytes = 0;
	// 重点
	ZeroMemory(&cn->overlapped, sizeof(OVERLAPPED));
	cn->optype = CONNECT_POSTED;
	BOOL bResult = gConnectEx(cn->s,
		(sockaddr *)&addrPeer,  // [in] 对方地址
		nLen,               // [in] 对方地址长度
		lpSendBuffer,       // [in] 连接后要发送的内容，这里不用
		dwSendDataLength,   // [in] 发送内容的字节数 ，这里不用
		&dwBytes,       // [out]发送了多少个字节，这里不用
		(OVERLAPPED *)&cn->overlapped);  // [in] 重叠IO结构
	if (!bResult)      // 返回值处理
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)   // 调用失败
		{
			TRACE(TEXT("ConnextEx error: %d/n"), WSAGetLastError());
			return -1;
		}
		else;// 操作未决（正在进行中 … ）
		{
			TRACE0("WSAGetLastError() == ERROR_IO_PENDING/n");// 操作正在进行中
		}

	}
	return 0;
}

int CManager::PostRecv(connect_key_t* lpCompletionKey, int size)
{
	overlapped_t* ov = (overlapped_t*)plx_palloc(pool, sizeof(overlapped_t));
	ZeroMemory(&ov->overlapped, sizeof(OVERLAPPED));
	ov->wsabuf.len = size;
	return 0;
}

int CManager::PostSend(connect_key_t* lpCompletionKey, int size)
{
	return 0;
}


int CManager::CloseConnection()
{
	return 0;
}

