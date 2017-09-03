/*/
文件：SelectServer.cpp
说明：

 此文件演示了如何使用select模型来建立服务器，难点是select的writefds在什么时候使用。
 好好看看代码就能很明白的了，可以说我写这些代码就是为了探索这个问题的！找了很多资料都找不到！！

 在这里我怀疑是否可以同时读写同一个SOCKET，结果发现是可以的，但是最好别这样做。因为会导致包的顺序不一致。

    这里说一下SELECT模型的逻辑：
 我们如果不使用select模型，在调用recv或者send时候会导致程序阻塞。如果使用了select
 就给我们增加了一层保护，就是说在调用了select函数之后，对处于读集合的socket进行recv操作
 是一定会成功的（这是操作系统给我们保证的）。对于判断SOCKET是否可写时也一样。
 而且就算不可读或不可写，使用了select也不会锁 死！因为 select 函数提供了超时！利用这个特性还可以
 做异步connect，也就是可以扫描主机，看哪个主机开了服务（远程控制软件经常这样干哦！）

 我们如何利用这种逻辑来设计我们的server呢？
 这里用的方法是建立一个SocketInfo，这个SocketInfo包括了对Socket当前进行的操作，我把它分为：
 {RecvCmd, RecvData, ExecCmd} 一开始socket是处于一个RecvCmd的状态，
 然后取到了CMD（也就是取到了指令，可想象一下CPU得到了指令后干什么），然后就要取数据了，取得指令
 知道要干什么，取得了数据就可以实际开始干了。实际开始干就是ExecCmd，在这个状态之后都是需要
 发送数据的了，所以把他们都放在判断SOCKET可写下面<就是 if(FD_ISSET(vecSocketInfo[i].sock, &fdWrite)) >，
 即当Socket可写就可以发送信息给客户端了。

 发送的根本协议是这样的：先发一个SCommand的结构体过去，这个结构体说明了指令和数据的长度。
 然后就根据这个长度接收数据。最后再给客户端做出相应的响应！

    根据这种代码结构，可以很方便的添加新的功能。

   错误处理做得不太好，以后再补充了。

 其他的如注释，结构，命名等的编码规范都用了个人比较喜欢的方式。

输出：
 ../Bin/SelectServer.exe

用法：
 直接启动就可以了

Todo:
 下一步首先完成各个SOCKET的模型，然后公开自己的研究代码。
 功能方面就是：
 1、服务器可以指定共享文件夹
 2、客户端可以列出服务器共享了哪些文件
 3、客户端可以列出哪些用户在线，并可以发命令和其他用户聊天
 4、加上界面
/*/
#include "StdAfx.h"
#include <winsock2.h>
#pragma comment(lib, "WS2_32")

#include <windows.h>

#pragma warning(disable: 4786)
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
using namespace std;

#include "../Include/CommonSocket.h"
#include "../Include/CommonCmd.h"

typedef struct tagSocketInfo
{
 SOCKET sock;
 ECurOp eCurOp;
 SCommand cmd;
 char *data;
}SSocketInfo;

// 登录用户的列表
map<string, SOCKET> g_LoginUsers;

// 注册用户的列表(用户名，密码)
map<string, string> g_RegUSers;

// 用于退出服务器
bool g_bExit = false;

void DoRecvCmd(vector<SSocketInfo> &vecSockInfo, int idx);
void DoRecvData(vector<SSocketInfo> &vecSockInfo, int idx);
void DoExecCmd(vector<SSocketInfo> &vecSockInfo, int idx);

bool DoAuthen(SOCKET sock, char *data, DWORD len);
bool DoGetFile(SOCKET sock, char *data, DWORD len);
bool DoRegister(SOCKET sock, char *data, DWORD len);

void GetRegUsers();

///////////////////////////////////////////////////////////////////////
//
// 函数名       : RemoveByIndex
// 功能描述     : 根据 index 来删除 VECTOR 里的元素
// 参数         : vector<T> &vec [in]
// 参数         : int nIdx   [in]
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
template<class T>
void EraseByIndex(vector<T> &vec, int nIdx)
{
 vector<T>::iterator it;
 it = vec.begin() + nIdx;
 vec.erase(it);
}

void main()
{
 InitWinsock();
 vector<SSocketInfo> vecSocketInfo;

 SOCKET sockListen = BindServer(PORT);
 ULONG NonBlock = 1;
 ioctlsocket(sockListen, FIONBIO, &NonBlock);
 
 SOCKET sockClient;

 GetRegUsers();

 FD_SET fdRead;
 FD_SET fdWrite;
 
 while(!g_bExit)
 {
  // 每次调用select之前都要把读集和写集清空
  FD_ZERO(&fdRead);
  FD_ZERO(&fdWrite);
  
  // 设置好读集和写集
  FD_SET(sockListen, &fdRead);
  for(int i = 0; i < vecSocketInfo.size(); i++)
  {
   FD_SET(vecSocketInfo[i].sock, &fdRead);
   FD_SET(vecSocketInfo[i].sock, &fdWrite);
  }

  // 调用select函数
  if(select(0, &fdRead, &fdWrite, NULL, NULL) == SOCKET_ERROR)
  {
   OutErr("select() Failed!");
   break;
  }

  // 说明可以接受连接了
  if(FD_ISSET(sockListen, &fdRead))
  {
   char szClientIP[50];
   sockClient = AcceptClient(sockListen, szClientIP);
   cout << szClientIP << " 连接上来" << endl;

   ioctlsocket(sockClient, FIONBIO, &NonBlock);

   SSocketInfo sockInfo;
   sockInfo.sock = sockClient;
   sockInfo.eCurOp = RecvCmd;
   // 把接收到的这个socket加入自己的队列中
   vecSocketInfo.push_back(sockInfo);
  }

  for(int i = 0; i < vecSocketInfo.size(); i++)
  {
   // 如果可读
   if(FD_ISSET(vecSocketInfo[i].sock, &fdRead))
   {
    switch(vecSocketInfo[i].eCurOp)
    {
    case RecvCmd:
     DoRecvCmd(vecSocketInfo, i);
     break;

    case RecvData:
     DoRecvData(vecSocketInfo, i);
     break;
     
    default:
     break;
    }
   }

   // 如果可写
   if(FD_ISSET(vecSocketInfo[i].sock, &fdWrite))
   {
    switch(vecSocketInfo[i].eCurOp)
    {
    case ExecCmd:
     DoExecCmd(vecSocketInfo, i);
     break;
    
    default:
     break;
    }
   }
  }
 }
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : DoRecvCmd
// 功能描述     : 获取客户端传过来的cmd
// 参数         : vector<SSocketInfo> &vecSockInfo
// 参数         : int idx
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void DoRecvCmd(vector<SSocketInfo> &vecSockInfo, int idx)
{
 SSocketInfo *sockInfo = &vecSockInfo[idx];
 int nRet = RecvFix(sockInfo->sock, (char *)&(sockInfo->cmd), sizeof(sockInfo->cmd));

 // 如果用户正常登录上来再用 closesocket 关闭 socket 会返回0 
 // 如果用户直接关闭程序会返回 SOCKET_ERROR，强行关闭
 if(nRet == SOCKET_ERROR || nRet == 0)
 {
  OutMsg("客户端已退出。");
  closesocket(sockInfo->sock);
  sockInfo->sock = INVALID_SOCKET;     
  EraseByIndex(vecSockInfo, idx);
  return;
 }
 sockInfo->eCurOp = RecvData;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : DoRecvData
// 功能描述     : DoRecvCmd 已经获得了指令，接下来就要获得执行指令所需要的数据
// 参数         : vector<SSocketInfo> &vecSockInfo
// 参数         : int idx
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void DoRecvData(vector<SSocketInfo> &vecSockInfo, int idx)
{
 SSocketInfo *sockInfo = &vecSockInfo[idx];
 // 为数据分配空间，分配多一位用来放最后的0
 sockInfo->data = new char[sockInfo->cmd.DataSize + 1];
 memset(sockInfo->data, 0, sockInfo->cmd.DataSize + 1);
 
 // 接收数据
 int nRet = RecvFix(sockInfo->sock, sockInfo->data, sockInfo->cmd.DataSize);
 if(nRet == SOCKET_ERROR || nRet == 0)
 {
  OutMsg("客户端已退出。");
  closesocket(sockInfo->sock);
  sockInfo->sock = INVALID_SOCKET;     
  EraseByIndex(vecSockInfo, idx);
  return;
 }
  
 sockInfo->eCurOp = ExecCmd;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : DoExecCmd
// 功能描述     : 指令和执行指令所需数据都已经准备好了，接下来就可以执行命令
// 参数         : vector<SSocketInfo> &vecSockInfo
// 参数         : int idx
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void DoExecCmd(vector<SSocketInfo> &vecSockInfo, int idx)
{
 SSocketInfo *sockInfo = &vecSockInfo[idx];
 switch(sockInfo->cmd.CommandID) 
 {
 case CMD_AUTHEN:
  DoAuthen(sockInfo->sock, sockInfo->data, sockInfo->cmd.DataSize);
   break;
 case CMD_GETFILE:
  DoGetFile(sockInfo->sock, sockInfo->data, sockInfo->cmd.DataSize);
  break;
 case CMD_REGISTER:
  DoRegister(sockInfo->sock, sockInfo->data, sockInfo->cmd.DataSize);
  break;
 default:
  break;
 }

 // 执行完命令后就设置回接收指令状态
 sockInfo->eCurOp = RecvCmd;
}

///////////////////////////////////////////////////////////////////////
//
// 函数名       : DoAuthen
// 功能描述     : 对用户名和密码做验证
// 参数         : SOCKET sock
// 参数         : char *data
// 参数         : DWORD len
// 返回值       : bool 
//
///////////////////////////////////////////////////////////////////////
bool DoAuthen(SOCKET sock, char *data, DWORD len)
{
 // 取得用户名和密码的字符串
 // 格式为 "dyl 123"

 char *pBuf = data;
 int nIdx = 0;
 char szName[10];
 memset(szName, 0, 10);
 char szPass[10];
 memset(szPass, 0, 10);
 
 while (*pBuf != ' ')
 {
  szName[nIdx++] = *pBuf++;
 }
 szName[nIdx] = '/0';

 *pBuf++;

 nIdx = 0;
 while (*pBuf != '/0')
 {
  szPass[nIdx++] = *pBuf++;
 }
 szPass[nIdx] = '/0';


 char szSend[30];
 memset(szSend, 0, 30);
 bool bUserExist = false;

 if( g_RegUSers.find(string(szName)) != g_RegUSers.end() )
 {
  if(strcmp(g_RegUSers[szName].c_str(), szPass) == 0)
  {
   strcpy(szSend, "UP OK!");
   g_LoginUsers[szName] = sock;
  }
  else
  {
   strcpy(szSend, "P Err!");
  }  
 }
 else
 {
 // 不存在这个用户
  strcpy(szSend, "U Err!");
 }
 
 int nRet = SendFix(sock, szSend, strlen(szSend));

 if(nRet == SOCKET_ERROR)
  return false;

 // 执行完了就释放data
 delete []data;

 return true;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : DoGetFile
// 功能描述     : 为用户提供文件
// 参数         : SOCKET sock
// 参数         : char *data
// 参数         : DWORD len
// 返回值       : bool 
//
///////////////////////////////////////////////////////////////////////
bool DoGetFile(SOCKET sock, char *data, DWORD len)
{
 // 打开文件，判断文件是否存在
 HANDLE hFile = CreateFile(data, GENERIC_READ, FILE_SHARE_READ, 
  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
 
 if(hFile == INVALID_HANDLE_VALUE)
 {
  OutMsg("文件不存在！");
  DWORD dwSize = 0;
  SendFix(sock, (char *)&dwSize, sizeof(dwSize));
  return false;
 }
 else
 {// 发送文件信息

  // 发送文件大小，发送过去
  DWORD dwFileSize = GetFileSize(hFile, NULL);
  int nRet = SendFix(sock, (char *)&dwFileSize, sizeof(dwFileSize));
  if(nRet == SOCKET_ERROR)
   return false;
  
  // 读文件记录并发送
  DWORD nLeft = dwFileSize;
  char szBuf[1024];
  DWORD nCurrRead = 0;
  while(nLeft > 0)
  {
   if(!ReadFile(hFile, szBuf, 1024, &nCurrRead, NULL))
   {
    OutErr("ReadFile failed!");
    return false;
   }
   SendFix(sock, szBuf, nCurrRead);
   nLeft -= nCurrRead;
  }
  
  CloseHandle(hFile);
 }
 
 delete []data;
 return true;
}

bool DoRegister(SOCKET sock, char *data, DWORD len)
{
 // 取得用户名和密码的字符串
 // 格式为 "dyl 123"

 bool bReturn = true;
 char *pBuf = data;
 int nIdx = 0;
 char szName[10];
 memset(szName, 0, 10);
 char szPass[20];
 memset(szPass, 0, 20);
 
 while (*pBuf != ' ')
 {
  szName[nIdx++] = *pBuf++;
 }
 szName[nIdx] = '/0';

 *pBuf++;

 nIdx = 0;
 while (*pBuf != '/0')
 {
  szPass[nIdx++] = *pBuf++;
 }
 szPass[nIdx] = '/0';

 char szSend[30];
 memset(szSend, 0, 30); 

 HANDLE hFile = CreateFile("Users.lst", GENERIC_WRITE, FILE_SHARE_READ, NULL, 
  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
 if(hFile == INVALID_HANDLE_VALUE)
 {
  hFile = CreateFile("Users.lst", GENERIC_WRITE, FILE_SHARE_READ, NULL, 
   CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hFile == INVALID_HANDLE_VALUE) 
  {
   OutMsg("创建文件Users.lst失败！");
   strcpy(szSend, "REG ERR!");
   bReturn = false;
  }
  else
  {
   // 在开始加
   SetFilePointer(hFile, 0, 0, FILE_BEGIN);
   DWORD dwWritten = 0;
   if(!WriteFile(hFile, szName, 10, &dwWritten, NULL))
   {
    OutMsg("WriteFile failed!");
    strcpy(szSend, "REG ERR!");
    bReturn = false;
   }
   if(!WriteFile(hFile, szPass, 20, &dwWritten, NULL))
   {
    OutMsg("WriteFile failed!");
    strcpy(szSend, "REG ERR!");
    bReturn = false;
   }
   
   CloseHandle(hFile);

   // 读回到已注册用户列表中
   GetRegUsers();

   strcpy(szSend, "REG OK!");
  }
 }
 else
 {
  // 移动到最后追加
  SetEndOfFile(hFile);
  DWORD dwWritten = 0;
  if(!WriteFile(hFile, szName, 10, &dwWritten, NULL))
  {
   OutMsg("WriteFile failed!");
   strcpy(szSend, "REG ERR!");
   bReturn = false;
  }
  if(!WriteFile(hFile, szPass, 20, &dwWritten, NULL))
  {
   OutMsg("WriteFile failed!");
   strcpy(szSend, "REG ERR!");
   bReturn = false;
  }

  CloseHandle(hFile);

  // 读回到已注册用户列表中
  GetRegUsers();

  strcpy(szSend, "REG OK!");  
 }
 int nRet = SendFix(sock, szSend, strlen(szSend));
 if(nRet == SOCKET_ERROR)
  bReturn = false;

 // 执行完了就释放data
 delete []data; 

 return bReturn;
}

void GetRegUsers()
{
 g_RegUSers.clear();

 char szName[10];
 char szPwd[20];
 
 HANDLE hFile = CreateFile("Users.lst", GENERIC_READ, FILE_SHARE_READ, NULL, 
  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
 if(hFile == INVALID_HANDLE_VALUE)
 {
  OutMsg("用户列表不存在！");
 }
 else
 {
  DWORD dwFileSize = 0;
  dwFileSize = GetFileSize(hFile, NULL);
  
  SetFilePointer(hFile, 0, 0, FILE_BEGIN);

  DWORD dwRead = 0;
  
  
  DWORD dwLeft = dwFileSize;
  while(dwLeft > 0)
  {
   memset(szName, 0, 10);
   memset(szPwd, 0, 20);
   if(!ReadFile(hFile, szName, 10, &dwRead, NULL))
   {
    DWORD dwErr = GetLastError();
    OutMsg("ReadFile failed!");
   }
   dwLeft -= dwRead;
   if(!ReadFile(hFile, szPwd, 20, &dwRead, NULL))
   {
    DWORD dwErr = GetLastError();
    OutMsg("ReadFile failed!");
   }
   dwLeft -= dwRead;
   g_RegUSers[szName] = szPwd;
  } 
 }

 CloseHandle(hFile);
}