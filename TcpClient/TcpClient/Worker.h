#pragma once
#include "Poolx.h"
#include "Common.h"
#include <winsock2.h>
#include <MSWSock.h>
#pragma comment(lib,"Mswsock.lib")
#define EXIT_CODE NULL
#define MAX_ACTIVE_CONNECT 1024
#define POOL_SIZE    4095
class CWorker
{
public:
	CWorker(HANDLE hIocp);
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
private:
	// worker thread fun
	static DWORD WorkerThread(CWorker* lpVoid);
	int PostRecv(connect_key_t* lpCompletionKey, int size);
	int PostSend(connect_key_t* lpCompletionKey, overlapped_t* lpOverlapped);
private:
	HANDLE m_hIocp;
	// worker thread handle
	HANDLE m_handle;
	HANDLE m_phIocp;
	plx_pool_t* pool;
public:
	HANDLE Init();
	int Stop();
};

