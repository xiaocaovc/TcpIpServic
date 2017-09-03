/*************************************
Funtion:Create Server

**************************************/

#include "stdafx.h"
#include "Server.h"
#define EXIT_CODE NULL

LPFN_ACCEPTEX gAcceptEx;
LPFN_GETACCEPTEXSOCKADDRS gGetAcceptExSockAddrs;
LPFN_DISCONNECTEX gDisconnectEx; // 扩展函数DisconnectEx的指针

CServer::CServer()
	:m_hIocp(NULL)
	, m_nWorker(0)
	,m_handle(NULL)
	, m_actsocket(NULL)
	, m_lpWorkerArray(NULL)
	,keypool(NULL)
{
	// 初始化临界区
	InitializeCriticalSection(&m_cs);
}


CServer::~CServer()
{
	WSACleanup();
	// 删除临界区
	DeleteCriticalSection(&m_cs);
	if (keypool)
	{
		plx_destroy_pool(keypool);
	}
	if (m_lpWorkerArray)
	{
		delete[] m_lpWorkerArray;
	}
}

// 获得函数地址
int CServer::GetFunAdr(SOCKET socket)
{
	// AcceptEx 和 GetAcceptExSockaddrs 的GUID，用于导出函数指针
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	// 获取AcceptEx函数指针
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx,
		sizeof(GuidAcceptEx),
		&gAcceptEx,
		sizeof(LPFN_ACCEPTEX),
		&dwBytes,
		NULL,
		NULL))
	{
		TRACE("WSAIoctl 未能获取AcceptEx函数指针。错误代码: %d!!", WSAGetLastError());
		return -1;
	}
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	// 获取GetAcceptExSockAddrs函数指针，也是同理
	if (SOCKET_ERROR == WSAIoctl(socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs),
		&gGetAcceptExSockAddrs,
		sizeof(LPFN_GETACCEPTEXSOCKADDRS),
		&dwBytes,
		NULL,
		NULL))
	{
		TRACE("WSAIoctl 未能获取GuidGetAcceptExSockAddrs函数指针。错误代码: %d!!", WSAGetLastError());
		return -1;
	}
	GUID GuidDisconnectEx = WSAID_DISCONNECTEX;
	if (SOCKET_ERROR == WSAIoctl(socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidDisconnectEx,
		sizeof(GuidDisconnectEx),
		&gDisconnectEx,
		sizeof(LPFN_DISCONNECTEX),
		&dwBytes,
		NULL,
		NULL))
	{
		TRACE("WSAIoctl 未能获取GuidDisconnectEx函数指针。错误代码: %d!!", WSAGetLastError());
		return -1;
	}
	return 0;
}

// 停止
int CServer::Stop()
{
	if (m_handle)
	{
		PostQueuedCompletionStatus(m_hIocp, 0, (DWORD)EXIT_CODE, NULL);
		CWorker::WaitThreadClose(m_handle);
	}
	return 0;
}

int CServer::CreateServer(unsigned short nPort, char* szIP)
{
	WSADATA wsaData;
	// 错误(一般都不可能出现)
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		TRACE("---初始化WinSock 2.2失败!---\n");
		return -1;
	}
	// 建立第一个完成端口
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_hIocp)
	{
		TRACE(_T("建立完成端口失败！错误代码: %d！"), WSAGetLastError());
		return -1;
	}

	// 建立监视Socket
	m_Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_Socket)
	{
		TRACE(_T("初始化监视Socket失败，错误代码: %d."), WSAGetLastError());
		return -1;
	}
	if (0 != GetFunAdr(m_Socket))
	{
		closesocket(m_Socket);
		return -1;
	}
	
	// 服务器地址信息，用于绑定Socket
	struct sockaddr_in ServerAddress;
	// 填充地址信息
	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	// 这里可以绑定任何可用的IP地址，或者绑定一个指定的IP地址
	if (szIP)
	{
		ServerAddress.sin_addr.s_addr = inet_addr(szIP);
	}
	else
	{
		ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	ServerAddress.sin_port = htons(nPort);
	// 绑定地址和端口
	if (SOCKET_ERROR == bind(m_Socket, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)))
	{
		TRACE("bind()函数执行错误！");
		return false;
	}
	// 开始进行监听
	if (SOCKET_ERROR == listen(m_Socket, SOMAXCONN))
	{
		TRACE("监听失败！错误代码: %d.", WSAGetLastError());
		return false;
	}
	// 绑定完成端口
	if (NULL == CreateIoCompletionPort((HANDLE)m_Socket, m_hIocp, (DWORD)(this), 0))
	{
		TRACE(_T("绑定 Listen Socket至完成端口失败！错误代码: %d！"), WSAGetLastError());
		return -1;
	}
	// 监视线程
	DWORD nThreadID = 0;
	m_handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ListenThread, (void *)this, 0, &nThreadID);
	if (!m_handle)
	{
		return -1;
	}
	keypool = plx_create_pool(1024);
	// 建立工作
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	int nWorker = si.dwNumberOfProcessors * 2;
	m_lpWorkerArray = new CWorker[nWorker];
	for (int i = 0; i < nWorker; i++)
	{
		if (0==m_lpWorkerArray[i].Run())
		{
			m_nWorker++;

		}
	}
	// 投送监听
	for (int i = 0; i < m_nWorker; i++)
	{
		PostAccept();
	}
	return 0;
}

DWORD CServer::ListenThread(CServer* lpVoid)
{
	net_completionkeyex_t* lpOverlapped = NULL;
	CServer* lpCompletionKey = NULL;
	DWORD dwBytesTransfered = 0;
	BOOL bReturn = FALSE;
	while (true)
	{
		bReturn = GetQueuedCompletionStatus(lpVoid->m_hIocp,
			&dwBytesTransfered, (PULONG_PTR)&lpCompletionKey, (LPOVERLAPPED*)&lpOverlapped,
			INFINITE);
		// 如果收到的是退出标志，则直接退出
		if (EXIT_CODE == (DWORD)lpCompletionKey)
		{
			break;
		}
		else
		{
			HANDLE handle = lpVoid->GetAcceptIocp();
			if (handle)
			{
				// 客户SOCKET绑定IOCP
				if (NULL == CreateIoCompletionPort((HANDLE)lpOverlapped->s,
					handle, (DWORD)lpOverlapped, 0))
				{
					TRACE("-----客户SOCKET绑定IOCP出现错误.错误代码:%d-----", GetLastError());
					lpVoid->PostAccept();
					continue;
				}
				// 传递lpKey
				PostQueuedCompletionStatus(handle, 1, (DWORD)lpOverlapped, (LPOVERLAPPED)lpOverlapped);
			}
			lpVoid->PostAccept();
		}
	}
	return 0;
}

bool CServer::PostAccept()
{
	net_completionkeyex_t* key=NULL;
	/*EnterCriticalSection(&m_cs);
	if (m_actsocket)
	{
		key = m_actsocket;
		m_actsocket = m_actsocket->next;
		ZeroMemory(&key->overlapped, sizeof(OVERLAPPED));
		gDisconnectEx(key->s, &key->overlapped, TF_REUSE_SOCKET, NULL);
	}
	LeaveCriticalSection(&m_cs);*/
	if (!key)
	{
		key = (net_completionkeyex_t*)plx_pcalloc(keypool, sizeof(net_completionkeyex_t));
		if (!key)
		{
			return false;
		}
		key->optype = ACCEPT_POSTED;
		key->s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	}
	if (!gAcceptEx(m_Socket,
		key->s,
		&key->buffer,
		0,
		sizeof(SOCKADDR_IN) + 16, 
		sizeof(SOCKADDR_IN) + 16,
		&key->dwbytes,
		&key->overlapped))
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			TRACE("-----投递 AcceptEx 请求失败，错误代码: %d-----", WSAGetLastError());
			return false;
		}
	}
	return true;
}


void CServer::AddNoActSocket(net_completionkeyex_t* act)
{
	EnterCriticalSection(&m_cs);
	/*act->next = m_actsocket;
	m_actsocket = act;*/
	LeaveCriticalSection(&m_cs);
}


HANDLE CServer::GetAcceptIocp()
{
	HANDLE hIocp=NULL;
	static int n = 0;
	for (int i = n;;)
	{
		if (hIocp = m_lpWorkerArray[i].GetHocp())
		{
			break;
		}
		if (++i==m_nWorker)
		{
			i = 0;
		}
		if (i == n)
		{
			break;
		}
	}
	return hIocp;
}
