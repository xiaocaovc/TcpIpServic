/////////////////////////////////////////////////////////////////////////////
// Name:        TcpServiceExp.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////
#pragma once
#ifdef TCPCLIENT_OP_API
#else
#ifdef _DEBUG
#pragma comment(lib,"TcpClientd.lib")
#else
#pragma comment(lib,"TcpClient.lib")
#endif
#define TCPCLIENT_OP_API extern "C" _declspec(dllimport)
#endif

typedef void(*BackFunc)(void* lpKey, void* lpdata, int optype, int len, void* lpobj);
TCPCLIENT_OP_API int cli_init();
TCPCLIENT_OP_API int cli_ConnectToServer(unsigned short nSvrPort, char* szSvrIP, BackFunc backfun, void* pObj);