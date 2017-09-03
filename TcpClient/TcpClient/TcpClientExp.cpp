/////////////////////////////////////////////////////////////////////////////
// Name:        TcpServiceExp.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#define TCPCLIENT_OP_API extern "C" _declspec(dllexport)
#include "TcpClientExp.h"
#include "Manager.h"
CManager man;
TCPCLIENT_OP_API int cli_init()
{
	return man.Init();
}
TCPCLIENT_OP_API int cli_ConnectToServer(unsigned short nSvrPort, char* szSvrIP, BackFunc backfun, void* pObj)
{
	return man.ConnectToServer(nSvrPort, szSvrIP,backfun,pObj);
}