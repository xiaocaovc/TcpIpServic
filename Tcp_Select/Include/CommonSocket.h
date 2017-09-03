/*/
文件: CommonSocket.h
说明: 
 实现了服务端和客户端一些公用的函数!
/*/

#ifndef __COMMONSOCKET_H__
#define __COMMONSOCKET_H__

#include <iostream>
using namespace std;

#define OutErr(a) cout << (a) << endl /
      << "出错代码：" << WSAGetLastError() << endl /
      << "出错文件：" << __FILE__ << endl  /
      << "出错行数：" << __LINE__ << endl /

#define OutMsg(a) cout << (a) << endl;

///////////////////////////////////////////////////////////////////////
//
// 函数名       : InitWinsock
// 功能描述     : 初始化WINSOCK
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void InitWinsock()
{
 // 初始化WINSOCK
 WSADATA wsd;
 if( WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
 {
  OutErr("WSAStartup()");
 }
}

///////////////////////////////////////////////////////////////////////
//
// 函数名       : ConnectServer
// 功能描述     : 连接SERVER
// 参数         : char *lpszServerIP IP地址
// 参数         : int nPort    端口
// 返回值       : SOCKET    SERVER 的 Socket
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

 
 // 如果给的是主机的名字而不是IP地址
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
// 函数名       : BindServer
// 功能描述     : 绑定端口
// 参数         : int nPort
// 返回值       : SOCKET 
//
///////////////////////////////////////////////////////////////////////
SOCKET BindServer(int nPort)
{
 // 创建socket 
 SOCKET sServer = socket(AF_INET, SOCK_STREAM, 0);

 // 绑定端口
 struct sockaddr_in servAddr;
 servAddr.sin_family = AF_INET;
 servAddr.sin_port = htons(nPort);
 servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

 if(bind(sServer, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
 {
  OutErr("bind Failed!");
  return NULL;
 }

 // 设置监听队列为200
 if(listen(sServer, 200) != 0)
 {
  OutErr("listen Failed!");
  return NULL;
 }
 return sServer;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : AcceptClient
// 功能描述     : 
// 参数         : SOCKET sServer [in]
// 参数         : LPSTR lpszIP  [out] 返回客户端的IP地址 
// 返回值       : SOCKET   [out] 返回客户端的socket
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
// 函数名       : RecvFix
// 功能描述     : 接收指定长度的数据，考虑非阻塞socket的情况
// 参数         : SOCKET socket [in]
// 参数         : char *data [in]
// 参数         : DWORD len  [in]
// 参数         : DWORD *retlen [out]
// 返回值       : bool 
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
// 函数名       : SendFix
// 功能描述     : 发送指定长度的数据，考虑非阻塞socket的情况
// 参数         : SOCKET socket
// 参数         : char *data
// 参数         : DWORD len
// 参数         : DWORD *retlen
// 返回值       : bool 
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
// 函数名       : SelectSend
// 功能描述     : 使用select模型来发送数据，没完成，所以注释掉了
// 参数         : SOCKET sock
// 参数         : FD_SET *wfds
// 参数         : char *data
// 参数         : DWORD len
// 返回值       : bool 
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
 // 如果是可以写的SOCKET，就一直写，直到返回WSAEWOULDBLOCK
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
// 函数名       : SelectRecv
// 功能描述     : 使用select模型来接收数据，没完成，所以注释掉了
// 参数         : SOCKET sock
// 参数         : FD_SET *rfds
// 参数         : char *data
// 参数         : DWORD len
// 返回值       : bool 
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