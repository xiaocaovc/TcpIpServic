#pragma once
#include <winsock2.h>
#include <MSWSock.h>
#include"NetWorker.h"
typedef struct _OVERLAPPEDE_ACCEPT
{
	OVERLAPPED      m_Overlapped; // �ص��ṹ
	OPERATION_TYPE  m_OpType;     // ������ʶ
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
	//�ȴ�ֹͣ�߳�
	void WaitThreadClose(HANDLE handle)
	{
		MSG msg;
		DWORD result;
		while (handle != NULL)
		{
			result = MsgWaitForMultipleObjects(1, &handle, false, INFINITE, QS_ALLINPUT);//INFINITE
			switch (result)
			{
			case WAIT_OBJECT_0: //�̵߳Ľ���
				CloseHandle(handle);
				handle = NULL;
				break; //break the loop
			case WAIT_OBJECT_0 + 1:
				////���߳���ʹ��GetSafeHwnd(),�����߳���GetForegroundWindow()��ô��ھ��
				PeekMessage(&msg, GetForegroundWindow(), 0, 0, PM_REMOVE);
				DispatchMessage(&msg);
				continue;
			default:
				return;/// unexpected failure 
			}
		}
	}
private:

	// ������Ӿ��
	HANDLE GetConnectHandle();
	// �����߳�
	static DWORD /*WINAPI*/ WorkerThread(CTcpService* lpVoid);
	// ��ɶ˿�,����Accept
	HANDLE m_hIocp;
	// ����SOCKET
	SOCKET m_Socket;
	LPFN_ACCEPTEX m_lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS m_lpfnGetAcceptExSockAddrs;
	// �����߳�
	HANDLE m_handle;
	OVERLAPPEDE_ACCEPT   m_Overlapped;
	OVERLAPPEDE_ACCEPT m_Recv;
	CWorker* m_lpWork;
	// Worker��Ŀ
	int m_nWorker;
	bool PostRecv(OVERLAPPEDE_ACCEPT* lpOverlapped)
	{
		lpOverlapped->m_OpType = RECV_POSTED;
		lpOverlapped->m_dwBytes = 0;
		lpOverlapped->m_dwFlags = 0;
		// ��ʼ��
		lpOverlapped->m_Overlapped.hEvent = NULL;
		lpOverlapped->m_wsaBuf[0].buf = lpOverlapped->lpData;
		lpOverlapped->m_wsaBuf[0].len = 10;
		// Ͷ��WSARecv����
		//ASSERT(INVALID_SOCKET!=m_Handle.m_Socket);
		ZeroMemory(&lpOverlapped->m_Overlapped, sizeof(lpOverlapped->m_Overlapped));
		int nRes = WSARecv(lpOverlapped->socket, &lpOverlapped->m_wsaBuf[0], 1, &lpOverlapped->m_dwBytes,
			&lpOverlapped->m_dwFlags, &lpOverlapped->m_Overlapped, NULL);
		if (SOCKET_ERROR == nRes)
		{
			nRes = WSAGetLastError();
			if (WSA_IO_PENDING != nRes)
			{
				TRACE("---����˽��մ���SOCKET:%d,�ͷ���Դ,�������:%d---\n", lpOverlapped->socket, nRes);
				return false;
			}
			return true;
		}
		return true;
	}
private:
	bool PostAccept();
public:
	// ��ʼ��������
	int Init(unsigned short nPort, char* szIP=NULL);
	bool Stop();
};

