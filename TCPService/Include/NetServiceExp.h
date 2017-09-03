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
#ifdef NETSERVICE_OP_API
#else
#ifdef _DEBUG
#pragma comment(lib,"TcpServiced.lib")
#else
#pragma comment(lib,"TcpService.lib")
#endif
#define NETSERVICE_OP_API extern "C" _declspec(dllimport)
#endif
NETSERVICE_OP_API int net_CreateServer(unsigned short nPort, char* szIP);
NETSERVICE_OP_API int net_StopServer();