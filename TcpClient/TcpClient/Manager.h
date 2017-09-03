#pragma once
#include"Worker.h"
#include <map>

class CManager
{
public:
	CManager();
	~CManager();
private:
	int GetFunAdr(SOCKET s);
	static DWORD ManThread(CManager* lpVoid);
private:
	// 管理线程句柄
	HANDLE m_handle;
	HANDLE m_hIocp;
	std::map<CWorker* , worker_data_t> workermap;
	std::map<CWorker*, worker_data_t>::iterator iter;
	connect_key_t* m_actqueue;
	plx_pool_t* pool;
public:
	int Init();
	int ConnectToServer(unsigned short nSvrPort, char* szSvrIP,BackFun backfun,void* pObj);
	int Stop();
	int PostRecv(connect_key_t* lpCompletionKey,int size);
	int PostSend(connect_key_t* lpCompletionKey, int size);
	int CloseConnection();
};

