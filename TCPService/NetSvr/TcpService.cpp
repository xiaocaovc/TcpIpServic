#include "stdafx.h"
#include "TcpService.h"
#pragma comment(lib,"ws2_32.lib")
#define EXIT_CODE NULL


char Buff[(sizeof(SOCKADDR_IN) + 16) * 2];
CTcpService::CTcpService()
	:m_handle(NULL)
	, m_lpWork(NULL)
	, m_nWorker(0)
{
	WSADATA wsaData;
	// 错误(一般都不可能出现)
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		TRACE("---初始化WinSock 2.2失败!---\n");
	}
}

// 释放
CTcpService::~CTcpService()
{
	if (m_lpWork)
	{
		delete[] m_lpWork;
	}
	WSACleanup();
}

// 停止
bool CTcpService::Stop()
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
bool CTcpService::PostAccept()
{
	// 准备参数
	DWORD dwBytes = 0;
	m_Overlapped.socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	ZeroMemory(&m_Overlapped.m_Overlapped, sizeof(OVERLAPPED));
	if (!m_lpfnAcceptEx(m_Socket, m_Overlapped.socket,
		&Buff, 0/*p_wbuf->len - ((sizeof(SOCKADDR_IN)+16)*2*/,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &m_Overlapped.m_Overlapped))
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			TRACE("-----投递 AcceptEx 请求失败，错误代码: %d-----", WSAGetLastError());
			return false;
		}
	}
	return true;
}
// 初始化服务器
int CTcpService::Init(unsigned short nPort, char* szIP)
{
	m_Recv.lpData = new char[1024];
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
		return false;
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
	// AcceptEx 和 GetAcceptExSockaddrs 的GUID，用于导出函数指针
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	// 获取AcceptEx函数指针
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(m_Socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx,
		sizeof(GuidAcceptEx),
		&m_lpfnAcceptEx,
		sizeof(LPFN_ACCEPTEX),
		&dwBytes,
		NULL,
		NULL))
	{
		TRACE("WSAIoctl 未能获取AcceptEx函数指针。错误代码: %d!!", WSAGetLastError());
		return -1;
	}
	//#ifdef _DEBUG
	// 获取GetAcceptExSockAddrs函数指针，也是同理
	if (SOCKET_ERROR == WSAIoctl(m_Socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs),
		&m_lpfnGetAcceptExSockAddrs,
		sizeof(LPFN_GETACCEPTEXSOCKADDRS),
		&dwBytes,
		NULL,
		NULL))
	{
		TRACE("WSAIoctl 未能获取GuidGetAcceptExSockAddrs函数指针。错误代码: %d!!", WSAGetLastError());
		return -1;
	}
	// 绑定完成端口
	if (NULL == CreateIoCompletionPort((HANDLE)m_Socket, m_hIocp,(DWORD)(this), 0))
	{
		TRACE(_T("绑定 Listen Socket至完成端口失败！错误代码: %d！"), WSAGetLastError());
		return -1;
	}
	DWORD nThreadID = 0;
	m_handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)WorkerThread, (void *)this, 0, &nThreadID);
	if (!m_handle)
	{
		return -1;
	}
	// 建立工作线程
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	m_nWorker = si.dwNumberOfProcessors * 2;
	m_lpWork = new CWorker[m_nWorker];
	for (int i = 0; i < m_nWorker; i++)
	{
		m_lpWork[i].Init();
	}
	PostAccept();
	
	return 0;
}

// Accept 建立链接
DWORD CTcpService::WorkerThread(CTcpService* lpVoid)
{
	OVERLAPPEDE_ACCEPT* lpOverlapped = NULL;
	CTcpService* lpCompletionKey = NULL;
	DWORD dwBytesTransfered = 0;
	BOOL bReturn = FALSE;
	while (true)
	{
		bReturn = GetQueuedCompletionStatus(lpVoid->m_hIocp,
			&dwBytesTransfered, (PULONG_PTR)&lpCompletionKey,(LPOVERLAPPED*) &lpOverlapped,
			INFINITE);
		// 如果收到的是退出标志，则直接退出
		if (EXIT_CODE == (DWORD)lpCompletionKey)
		{
			break;
		}
		else
		{

			COMPLETIONKEYEX* lpKey = new COMPLETIONKEYEX;
			lpKey->socket = lpOverlapped->socket;
			HANDLE handle = lpVoid->GetConnectHandle();
			if (handle)
			{
				// 客户SOCKET绑定IOCP
				if (NULL == CreateIoCompletionPort((HANDLE)lpKey->socket,
					handle, (DWORD)lpKey, 0))
				{
					TRACE("-----客户SOCKET绑定IOCP出现错误.错误代码:%d-----", GetLastError());
					lpVoid-> PostAccept();
					continue;
				}
				// 传递lpKey
				PostQueuedCompletionStatus(handle, 0, (DWORD)lpKey, (LPOVERLAPPED)NULL);
			}
			 lpVoid->PostAccept();
		}
	}
	return 0;
}


// 获得连接句柄
HANDLE CTcpService::GetConnectHandle()
{
	HANDLE handle = NULL;
	for (int i = 0; i < m_nWorker; i++)
	{
		if (handle = m_lpWork[i].GetIocp())
		{
			return handle;
		}
	}
	return NULL;
}
