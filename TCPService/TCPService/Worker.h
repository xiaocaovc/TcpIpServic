#pragma once
#include <winsock2.h>
#include <MSWSock.h>
#include"Poolx.h"

typedef enum net_operation_s
{
	NULL_POSTED,    // ���ڳ�ʼ����������
	ACCEPT_POSTED, // ��־Ͷ�ݵ�Accept����
	SEND_POSTED,   // ��־Ͷ�ݵ��Ƿ��Ͳ���
	RECV_POSTED,   // ��־Ͷ�ݵ��ǽ��ղ���
	WRITE_POSTED,  // ��־Ͷ�ݵ���д�����
	READ_POSTED,   // ��־Ͷ�ݵ��Ƕ�ȡ����

}net_operation_t;
typedef struct net_completionkeyex_s net_completionkeyex_t;
typedef struct net_overlappedex_s net_overlappedex_t;
struct net_completionkeyex_s
{
	OVERLAPPED      overlapped; // �ص��ṹ
	net_operation_t  optype;     // ������ʶ
	SOCKET s;
	char buffer[(sizeof(SOCKADDR_IN) + 16) * 2];
	DWORD dwbytes;
	DWORD dwflags;
	net_completionkeyex_t*   next; // 0,1
};
typedef struct net_overlappedex_s
{
	OVERLAPPED      overlapped; // �ص��ṹ
	net_operation_t  optype;     // ������ʶ
	WSABUF			wsabuf[1];
};
class CWorker
{
public:
	CWorker();
	~CWorker();
	//�ȴ�ֹͣ�߳�
	static void WaitThreadClose(HANDLE handle)
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
	bool PostRecv(net_completionkeyex_t* lpCompletionKey, int size)
	{
		DWORD dwbytes = 0;
		DWORD dwflags = 0;
		net_overlappedex_t* lpOverlapped = (net_overlappedex_t*)plx_palloc(pool,size + sizeof(net_overlappedex_t));
		lpOverlapped->optype = RECV_POSTED;
		// ��ʼ��
		lpOverlapped->wsabuf[0].buf = (char*)((char*)lpOverlapped + sizeof(net_overlappedex_t));
		lpOverlapped->wsabuf[0].len = size;
		// Ͷ��WSARecv����
		ZeroMemory(&lpOverlapped->overlapped, sizeof(OVERLAPPED));
		int nRes = WSARecv(lpCompletionKey->s, &lpOverlapped->wsabuf[0], 1, &dwbytes,
			&dwflags, &lpOverlapped->overlapped, NULL);
		if (SOCKET_ERROR == nRes)
		{
			nRes = WSAGetLastError();
			if (WSA_IO_PENDING != nRes)
			{
				TRACE("---����˽��մ���SOCKET:%d,�ͷ���Դ,�������:%d---\n", lpCompletionKey->s, nRes);
				return false;
			}
			return true;
		}
		return true;
	}
private:
	// �����߳̾��
	HANDLE m_handle;
	HANDLE m_hIocp;
	// �����߳�
	static DWORD WorkerThread(CWorker* lpVoid);
	plx_pool_t* pool;
public:
	int Run();
	HANDLE GetHocp();
	int Stop();
	int m_active;
};

