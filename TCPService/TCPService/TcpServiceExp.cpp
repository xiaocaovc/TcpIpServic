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
#define NETSERVICE_OP_API extern "C" _declspec(dllexport)
#include "NetServiceExp.h"
#include "Server.h"
CServer svr;
NETSERVICE_OP_API int net_CreateServer(unsigned short nPort, char* szIP)
{
	return svr.CreateServer(nPort, szIP);
}
NETSERVICE_OP_API int net_StopServer()
{
	return svr.Stop();
}