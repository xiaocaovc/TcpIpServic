#pragma once
#include <winsock2.h>
#include <MSWSock.h>
#pragma comment(lib,"Mswsock.lib")
#include"Worker.h"

class CServer
{
public:
	CServer();
	~CServer();
private:
	// ��ú�����ַ
	int GetFunAdr(SOCKET socket);
	static DWORD /*WINAPI*/ ListenThread(CServer* lpVoid);
	bool PostAccept();
private:
	// ����SOCKET
	SOCKET m_Socket;
	// ����IOCP
	HANDLE m_hIocp;
	// Worker
	CWorker* m_lpWorkerArray;
	int m_nWorker;
	HANDLE m_handle;
	plx_pool_t* keypool;
	net_completionkeyex_t* m_actsocket;
	// �ٽ���
	CRITICAL_SECTION m_cs;
public:
	int CreateServer(unsigned short nPort, char* szIP=NULL);
	// ֹͣ
	int Stop();
	void AddNoActSocket(net_completionkeyex_t* act);
	HANDLE GetAcceptIocp();
};

