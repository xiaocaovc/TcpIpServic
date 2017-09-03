/*************************************
Funtion:Create Server

**************************************/

#include "stdafx.h"
#include "Server.h"
#define EXIT_CODE NULL

LPFN_ACCEPTEX gAcceptEx;
LPFN_GETACCEPTEXSOCKADDRS gGetAcceptExSockAddrs;
LPFN_DISCONNECTEX gDisconnectEx; // ��չ����DisconnectEx��ָ��

CServer::CServer()
	:m_hIocp(NULL)
	, m_nWorker(0)
	,m_handle(NULL)
	, m_actsocket(NULL)
	, m_lpWorkerArray(NULL)
	,keypool(NULL)
{
	// ��ʼ���ٽ���
	InitializeCriticalSection(&m_cs);
}


CServer::~CServer()
{
	WSACleanup();
	// ɾ���ٽ���
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

// ��ú�����ַ
int CServer::GetFunAdr(SOCKET socket)
{
	// AcceptEx �� GetAcceptExSockaddrs ��GUID�����ڵ�������ָ��
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	// ��ȡAcceptEx����ָ��
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
		TRACE("WSAIoctl δ�ܻ�ȡAcceptEx����ָ�롣�������: %d!!", WSAGetLastError());
		return -1;
	}
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	// ��ȡGetAcceptExSockAddrs����ָ�룬Ҳ��ͬ��
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
		TRACE("WSAIoctl δ�ܻ�ȡGuidGetAcceptExSockAddrs����ָ�롣�������: %d!!", WSAGetLastError());
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
		TRACE("WSAIoctl δ�ܻ�ȡGuidDisconnectEx����ָ�롣�������: %d!!", WSAGetLastError());
		return -1;
	}
	return 0;
}

// ֹͣ
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
	// ����(һ�㶼�����ܳ���)
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		TRACE("---��ʼ��WinSock 2.2ʧ��!---\n");
		return -1;
	}
	// ������һ����ɶ˿�
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_hIocp)
	{
		TRACE(_T("������ɶ˿�ʧ�ܣ��������: %d��"), WSAGetLastError());
		return -1;
	}

	// ��������Socket
	m_Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_Socket)
	{
		TRACE(_T("��ʼ������Socketʧ�ܣ��������: %d."), WSAGetLastError());
		return -1;
	}
	if (0 != GetFunAdr(m_Socket))
	{
		closesocket(m_Socket);
		return -1;
	}
	
	// ��������ַ��Ϣ�����ڰ�Socket
	struct sockaddr_in ServerAddress;
	// ����ַ��Ϣ
	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	// ������԰��κο��õ�IP��ַ�����߰�һ��ָ����IP��ַ
	if (szIP)
	{
		ServerAddress.sin_addr.s_addr = inet_addr(szIP);
	}
	else
	{
		ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	ServerAddress.sin_port = htons(nPort);
	// �󶨵�ַ�Ͷ˿�
	if (SOCKET_ERROR == bind(m_Socket, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)))
	{
		TRACE("bind()����ִ�д���");
		return false;
	}
	// ��ʼ���м���
	if (SOCKET_ERROR == listen(m_Socket, SOMAXCONN))
	{
		TRACE("����ʧ�ܣ��������: %d.", WSAGetLastError());
		return false;
	}
	// ����ɶ˿�
	if (NULL == CreateIoCompletionPort((HANDLE)m_Socket, m_hIocp, (DWORD)(this), 0))
	{
		TRACE(_T("�� Listen Socket����ɶ˿�ʧ�ܣ��������: %d��"), WSAGetLastError());
		return -1;
	}
	// �����߳�
	DWORD nThreadID = 0;
	m_handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ListenThread, (void *)this, 0, &nThreadID);
	if (!m_handle)
	{
		return -1;
	}
	keypool = plx_create_pool(1024);
	// ��������
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
	// Ͷ�ͼ���
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
		// ����յ������˳���־����ֱ���˳�
		if (EXIT_CODE == (DWORD)lpCompletionKey)
		{
			break;
		}
		else
		{
			HANDLE handle = lpVoid->GetAcceptIocp();
			if (handle)
			{
				// �ͻ�SOCKET��IOCP
				if (NULL == CreateIoCompletionPort((HANDLE)lpOverlapped->s,
					handle, (DWORD)lpOverlapped, 0))
				{
					TRACE("-----�ͻ�SOCKET��IOCP���ִ���.�������:%d-----", GetLastError());
					lpVoid->PostAccept();
					continue;
				}
				// ����lpKey
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
			TRACE("-----Ͷ�� AcceptEx ����ʧ�ܣ��������: %d-----", WSAGetLastError());
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
