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
	// ����(һ�㶼�����ܳ���)
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		TRACE("---��ʼ��WinSock 2.2ʧ��!---\n");
	}
}

// �ͷ�
CTcpService::~CTcpService()
{
	if (m_lpWork)
	{
		delete[] m_lpWork;
	}
	WSACleanup();
}

// ֹͣ
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
	// ׼������
	DWORD dwBytes = 0;
	m_Overlapped.socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	ZeroMemory(&m_Overlapped.m_Overlapped, sizeof(OVERLAPPED));
	if (!m_lpfnAcceptEx(m_Socket, m_Overlapped.socket,
		&Buff, 0/*p_wbuf->len - ((sizeof(SOCKADDR_IN)+16)*2*/,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &m_Overlapped.m_Overlapped))
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			TRACE("-----Ͷ�� AcceptEx ����ʧ�ܣ��������: %d-----", WSAGetLastError());
			return false;
		}
	}
	return true;
}
// ��ʼ��������
int CTcpService::Init(unsigned short nPort, char* szIP)
{
	m_Recv.lpData = new char[1024];
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
		return false;
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
	// AcceptEx �� GetAcceptExSockaddrs ��GUID�����ڵ�������ָ��
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	// ��ȡAcceptEx����ָ��
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
		TRACE("WSAIoctl δ�ܻ�ȡAcceptEx����ָ�롣�������: %d!!", WSAGetLastError());
		return -1;
	}
	//#ifdef _DEBUG
	// ��ȡGetAcceptExSockAddrs����ָ�룬Ҳ��ͬ��
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
		TRACE("WSAIoctl δ�ܻ�ȡGuidGetAcceptExSockAddrs����ָ�롣�������: %d!!", WSAGetLastError());
		return -1;
	}
	// ����ɶ˿�
	if (NULL == CreateIoCompletionPort((HANDLE)m_Socket, m_hIocp,(DWORD)(this), 0))
	{
		TRACE(_T("�� Listen Socket����ɶ˿�ʧ�ܣ��������: %d��"), WSAGetLastError());
		return -1;
	}
	DWORD nThreadID = 0;
	m_handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)WorkerThread, (void *)this, 0, &nThreadID);
	if (!m_handle)
	{
		return -1;
	}
	// ���������߳�
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

// Accept ��������
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
		// ����յ������˳���־����ֱ���˳�
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
				// �ͻ�SOCKET��IOCP
				if (NULL == CreateIoCompletionPort((HANDLE)lpKey->socket,
					handle, (DWORD)lpKey, 0))
				{
					TRACE("-----�ͻ�SOCKET��IOCP���ִ���.�������:%d-----", GetLastError());
					lpVoid-> PostAccept();
					continue;
				}
				// ����lpKey
				PostQueuedCompletionStatus(handle, 0, (DWORD)lpKey, (LPOVERLAPPED)NULL);
			}
			 lpVoid->PostAccept();
		}
	}
	return 0;
}


// ������Ӿ��
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
