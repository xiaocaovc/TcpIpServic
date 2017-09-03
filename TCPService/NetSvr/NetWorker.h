#pragma once
#include <winsock2.h>
#define BUF_LEN 1024*2
typedef enum _OPERATION_TYPE
{
	NULL_POSTED,    // ���ڳ�ʼ����������
	ACCEPT_POSTED, // ��־Ͷ�ݵ�Accept����
	SEND_POSTED,   // ��־Ͷ�ݵ��Ƿ��Ͳ���
	RECV_POSTED,   // ��־Ͷ�ݵ��ǽ��ղ���
	WRITE_POSTED,  // ��־Ͷ�ݵ���д�����
	READ_POSTED,   // ��־Ͷ�ݵ��Ƕ�ȡ����

}OPERATION_TYPE;
typedef struct _COMPLETIONKEYEX
{
	SOCKET socket;
	bool bBusy;
	_COMPLETIONKEYEX* next;
}COMPLETIONKEYEX;
typedef struct _COMPLETIONKEYEX_POOL
{
	SOCKET socket;
	bool bBusy;
	_COMPLETIONKEYEX* next;
}COMPLETIONKEYEX_POOL;
typedef struct _OVERLAPPEDEX
{
	OVERLAPPED      m_Overlapped; // �ص��ṹ
	OPERATION_TYPE  m_OpType;     // ������ʶ
	SOCKET socket;
	char*  lpData;
	DWORD			m_dwFlags;
	DWORD			m_dwBytes;
	WSABUF			m_wsaBuf[1];
}OVERLAPPEDEX;
class CWorker
{
public:
	CWorker();
	~CWorker();
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
	bool PostRecv(COMPLETIONKEYEX* lpCompletionKey, OVERLAPPEDEX* lpOverlapped)
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
		int nRes = WSARecv(lpCompletionKey->socket, &lpOverlapped->m_wsaBuf[0], 1, &lpOverlapped->m_dwBytes,
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
	// �����߳�
	static DWORD /*WINAPI*/ WorkerThread(CWorker* lpVoid);
	// ��ɶ˿�,����Recv Send
	HANDLE m_hIocp;
	// �����߳�
	HANDLE m_handle;
	OVERLAPPEDEX m_Overlapped;
public:
	// ֹͣ
	bool Stop();
	int Init();
	// ���IOCP���
	HANDLE GetIocp();
	// ������
	int m_nConnect;
};

