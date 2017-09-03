/*/
�ļ�: CommonSocket.h
˵��: 
 ʵ���˷���˺Ϳͻ���һЩ���õĺ���!
/*/

#ifndef __COMMONSOCKET_H__
#define __COMMONSOCKET_H__

#include <iostream>
using namespace std;

#define OutErr(a) cout << (a) << endl /
      << "������룺" << WSAGetLastError() << endl /
      << "�����ļ���" << __FILE__ << endl  /
      << "����������" << __LINE__ << endl /

#define OutMsg(a) cout << (a) << endl;

///////////////////////////////////////////////////////////////////////
//
// ������       : InitWinsock
// ��������     : ��ʼ��WINSOCK
// ����ֵ       : void 
//
///////////////////////////////////////////////////////////////////////
void InitWinsock()
{
 // ��ʼ��WINSOCK
 WSADATA wsd;
 if( WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
 {
  OutErr("WSAStartup()");
 }
}

///////////////////////////////////////////////////////////////////////
//
// ������       : ConnectServer
// ��������     : ����SERVER
// ����         : char *lpszServerIP IP��ַ
// ����         : int nPort    �˿�
// ����ֵ       : SOCKET    SERVER �� Socket
//
///////////////////////////////////////////////////////////////////////
SOCKET ConnectServer(char *lpszServerIP, int nPort, ULONG NonBlock)
{
 SOCKET sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
 //ioctlsocket(sServer, FIONBIO, &NonBlock);
 
 struct hostent *pHost = NULL;
 struct sockaddr_in servAddr;
 servAddr.sin_family = AF_INET;
 servAddr.sin_port = htons(nPort);
 servAddr.sin_addr.s_addr = inet_addr(lpszServerIP);

 
 // ������������������ֶ�����IP��ַ
 if(servAddr.sin_addr.s_addr == INADDR_NONE)
 {
  pHost = gethostbyname( lpszServerIP );
  if(pHost == NULL)
  {
   OutErr("gethostbyname Failed!");
   return NULL;
  }
  memcpy(&servAddr.sin_addr, pHost->h_addr_list[0], pHost->h_length);
 }

 int nRet = 0;
 nRet = connect(sServer, (struct sockaddr *)&servAddr, sizeof(servAddr));
 if( nRet == SOCKET_ERROR )
 {
  OutErr("connect failed!");
  return NULL;
 }
  
 return sServer;
}

///////////////////////////////////////////////////////////////////////
//
// ������       : BindServer
// ��������     : �󶨶˿�
// ����         : int nPort
// ����ֵ       : SOCKET 
//
///////////////////////////////////////////////////////////////////////
SOCKET BindServer(int nPort)
{
 // ����socket 
 SOCKET sServer = socket(AF_INET, SOCK_STREAM, 0);

 // �󶨶˿�
 struct sockaddr_in servAddr;
 servAddr.sin_family = AF_INET;
 servAddr.sin_port = htons(nPort);
 servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

 if(bind(sServer, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
 {
  OutErr("bind Failed!");
  return NULL;
 }

 // ���ü�������Ϊ200
 if(listen(sServer, 200) != 0)
 {
  OutErr("listen Failed!");
  return NULL;
 }
 return sServer;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : AcceptClient
// ��������     : 
// ����         : SOCKET sServer [in]
// ����         : LPSTR lpszIP  [out] ���ؿͻ��˵�IP��ַ 
// ����ֵ       : SOCKET   [out] ���ؿͻ��˵�socket
//
///////////////////////////////////////////////////////////////////////
SOCKET AcceptClient(SOCKET sListen, LPSTR lpszIP)
{
 struct sockaddr_in cliAddrTmp;
 int cliAddrSize = sizeof(struct sockaddr_in);
 SOCKET sClient = accept(sListen, (struct sockaddr *)&cliAddrTmp, &cliAddrSize);
 if(sClient == INVALID_SOCKET)
 {
  OutErr("accept failed!");
  return NULL;
 }
 sprintf(lpszIP, "%s", inet_ntoa(cliAddrTmp.sin_addr));//cliAddrTmp.sin_port

 return sClient;
}

///////////////////////////////////////////////////////////////////////
//
// ������       : RecvFix
// ��������     : ����ָ�����ȵ����ݣ����Ƿ�����socket�����
// ����         : SOCKET socket [in]
// ����         : char *data [in]
// ����         : DWORD len  [in]
// ����         : DWORD *retlen [out]
// ����ֵ       : bool 
//
///////////////////////////////////////////////////////////////////////
int RecvFix(SOCKET socket, char *data, DWORD len)
{
 int retlen = 0;
 int nLeft = len;
 int nRead = 0;
 char *pBuf = data;
 while(nLeft > 0)
 {
  nRead = recv(socket, pBuf, nLeft, 0);
  if(nRead == SOCKET_ERROR || nRead == 0)
  {
   if(WSAEWOULDBLOCK == WSAGetLastError())
    continue;
   else
    return nRead;
  }
  
  nLeft -= nRead;
  retlen += nRead;
  pBuf += nRead;
 }
 return nRead;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : SendFix
// ��������     : ����ָ�����ȵ����ݣ����Ƿ�����socket�����
// ����         : SOCKET socket
// ����         : char *data
// ����         : DWORD len
// ����         : DWORD *retlen
// ����ֵ       : bool 
//
///////////////////////////////////////////////////////////////////////
int SendFix(SOCKET socket, char *data, DWORD len)
{
 int retlen = 0;
 int nLeft = len;
 int nWritten = 0;
 const char *pBuf = data;
 while(nLeft > 0)
 {
  nWritten = send(socket, data, nLeft, 0);
  if(nWritten == SOCKET_ERROR || nWritten == 0)
  {
   if(WSAEWOULDBLOCK == WSAGetLastError())
    continue;
   else
    return nWritten;
  }

  
  nLeft -= nWritten;
  retlen += nWritten;
  pBuf += nWritten;
 }
 return nWritten;
}


/*
///////////////////////////////////////////////////////////////////////
//
// ������       : SelectSend
// ��������     : ʹ��selectģ�����������ݣ�û��ɣ�����ע�͵���
// ����         : SOCKET sock
// ����         : FD_SET *wfds
// ����         : char *data
// ����         : DWORD len
// ����ֵ       : bool 
//
///////////////////////////////////////////////////////////////////////
bool SelectSend(SOCKET sock, FD_SET *wfds, char *data, DWORD len)
{
 FD_ZERO(wfds);
 FD_SET(sock, wfds);
 
 if(select(0, NULL, wfds, NULL, NULL) == SOCKET_ERROR)
 {
  OutErr("select() Failed!");
  return false;
 }
 // ����ǿ���д��SOCKET����һֱд��ֱ������WSAEWOULDBLOCK
 if( FD_ISSET(sock, wfds) )
 {
  int nLeft = len;
  while(nLeft > 0)
  {
   int nRet = send(sock, data, len, 0);
   if(nRet == SOCKET_ERROR)
    return false;
   nLeft -= nRet;
  }
 }

 return true;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : SelectRecv
// ��������     : ʹ��selectģ�����������ݣ�û��ɣ�����ע�͵���
// ����         : SOCKET sock
// ����         : FD_SET *rfds
// ����         : char *data
// ����         : DWORD len
// ����ֵ       : bool 
//
///////////////////////////////////////////////////////////////////////
bool SelectRecv(SOCKET sock, FD_SET *rfds, char *data, DWORD len)
{
 FD_ZERO(rfds);
 FD_SET(sock, rfds);
 
 if(select(0, rfds, NULL, NULL, NULL) == SOCKET_ERROR)
 {
  OutErr("select() Failed!");
  return false;
 }
 
 if( FD_ISSET(sock, rfds) )
 {
  int nLeft = len;
  while(nLeft > 0)
  {
   int nRet = recv(sock, data, len, 0);
   if(nRet == SOCKET_ERROR)
    return false;
   nLeft -= nRet;
  }
 }
 return true;
}
*/


#endif //__COMMONSOCKET_H__