/*/
文件：SocketClient.cpp

说明：
 此文件是作为测试的客户端，实现了登录和取文件的功能。
 和服务端的交互就是采用了发送命令、数据长度，然后发送具体的数据这样的顺序。
 详细可看服务端的说明。

 基本逻辑是这样的，客户端要先登录服务端，然后登录成功之后，才能进行相应的操作。

 错误处理做得不太好，以后再补充了。

 其他的如注释，结构，命名等的编码规范都用了个人比较喜欢的方式。

输出：
 ../Bin/SocketClient.exe

用法：
 可以 SocketClient Server_IP
 或者直接启动SocketClient，会提示你输入服务端的IP

Todo:
 下一步首先完成各个SOCKET的模型，然后公开自己的研究代码。
 功能方面就是：
 1、服务器可以指定共享文件夹
 2、客户端可以列出服务器共享了哪些文件
 3、客户端可以列出哪些用户在线，并可以发命令和其他用户聊天
/*/
#include "StdAfx.h"
#include <winsock2.h>
#pragma comment(lib, "WS2_32")

#include <iostream>
using namespace std;

#include <stdlib.h>

#include "../Include/CommonSocket.h"
#include "../Include/CommonCmd.h"

bool g_bAuth = false;

void GetFile(SOCKET sock);
bool Auth(SOCKET sock, char *szName, char *szPwd);
bool RegisterUser(SOCKET sock, char *szName, char *szPwd);


///////////////////////////////////////////////////////////////////////
//
// 函数名       : Usage
// 功能描述     : 提示程序用法
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void Usage() 
{ 
 printf("*******************************************/n"); 
 printf("Socket Client                            /n"); 
 printf("Written by DYL                         /n"); 
 printf("Email: dylgsy@163.com                 /n"); 
 printf("Usage: SocketClient.exe Server_IP          /n"); 
 printf("*******************************************/n"); 
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : Menu
// 功能描述     : 选择服务的界面
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void Menu()
{
 system("cls");
 printf("********************************************/n"); 
 printf("请选择操作：        /n/n"); 
 printf("1、取得文件         /n"); 
 printf("2、退出          /n"); 
 printf("********************************************/n"); 
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : LoginMenu
// 功能描述     : 用户登录的界面
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void LoginMenu()
{
 cout << "请按任意键继续操作．" <<endl;
 getchar();
 system("cls");
 printf("********************************************/n"); 
 printf("请选择操作：        /n/n"); 
 printf("1、登录          /n"); 
 printf("2、注册          /n"); 
 printf("3、退出          /n"); 
 printf("********************************************/n"); 
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : Login
// 功能描述     : 用户登录的界面逻辑
// 参数         : SOCKET sock
// 返回值       : bool 
//
///////////////////////////////////////////////////////////////////////
bool Login(SOCKET sock)
{
 bool bGoOn = true;
 while(bGoOn)
 {
  LoginMenu();
  int nChoose = 0;
  cin >> nChoose;

  char szName[10];
  char szPwd[20];
  char szConfirmPwd[20];
  memset(szName, 0, 10);
  memset(szPwd, 0, 20);
  memset(szConfirmPwd, 0, 20);

  bool bGoOnLogin = true;

  switch(nChoose) 
  {
  case 1:
   while(bGoOnLogin)
   {
    cout << "请输入你的用户名：";
    cin >> szName;
    cout << "请输入你的密码：";
    cin >> szPwd;
    if(Auth(sock, szName, szPwd))
    {
     return true; 
    }
    else
    {
     char c;
     cout << "继续登录？y/n" << endl;
     cin >> c;
     switch(c) 
     {
     case 'y':
      bGoOnLogin = true;
      break;
     case 'n':
      bGoOnLogin = false;
      break;
     default:
      break;
     }
    }
   }
   break;
   
  case 2:
   cout << "请输入你的用户名：";
   cin >> szName;
   cout << "请输入你的密码：";
   cin >> szPwd;
   cout << "请再次输入你的密码：";
   cin >> szConfirmPwd;
   if(strcmp(szPwd, szConfirmPwd) != 0)
   {
    cout << "前后密码不一致" << endl;
   }
   else
   {
    if(!RegisterUser(sock, szName, szPwd))
    {
     cout << "注册用户失败!" << endl;
    }
   }
   break;

  case 3:
   bGoOn = false;
   return false;
  default:
   break;
  }
 }

 return false;
}

void main(int argc, char *argv[])
{
 system("cls");
 char szServerIP[20];
 memset(szServerIP, 0, 20);

 if(argc != 2)
 {
  cout << "请输入服务器IP：";
  cin >> szServerIP;
 }
 else
 {
  strcpy(szServerIP, argv[1]);
 }
 InitWinsock();
 SOCKET sockServer;
 sockServer = ConnectServer(szServerIP, PORT, 1);
 if(sockServer == NULL)
 {
  OutErr("连接服务器失败！");
  return;
 }
 else
 {
  OutMsg("已和服务器建立连接！");
 }

 // 要求用户登录
 if(!Login(sockServer))
  return;

 // 登录成功，让用户选择服务
 int nChoose = 0;
 bool bExit = false;
 while(!bExit)
 {
  Menu();
  cin >> nChoose;
  switch(nChoose)
  {
  case 1:  // 获取文件
   GetFile(sockServer);
   break;
  case 2:
   bExit = true;
   break;   
  default:
   break;
  }
 }
 shutdown(sockServer, SD_BOTH);
 closesocket(sockServer);
 
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : Auth
// 功能描述     : 用户登录认证
// 参数         : SOCKET sock
// 参数         : char *szName
// 参数         : char *szPwd
// 返回值       : bool 
//
///////////////////////////////////////////////////////////////////////
bool Auth(SOCKET sock, char *szName, char *szPwd)
{
 char szCmd[50];
 memset(szCmd, 0, 50);
 strcpy(szCmd, szName);
 strcat(szCmd, " ");
 strcat(szCmd, szPwd);
 
 SCommand cmd;
 cmd.CommandID = CMD_AUTHEN;
 cmd.DataSize = strlen(szCmd);

 int nRet;
 nRet = SendFix(sock, (char *)&cmd, sizeof(cmd));
 if(nRet == SOCKET_ERROR)
 {
  OutErr("SendFix() failed!");
  return false;
 }
 else
 {
  SendFix(sock, szCmd, strlen(szCmd));
  char szBuf[10];
  memset(szBuf, 0, 10);
  recv(sock, szBuf, 10, 0);
  if(strcmp(szBuf, "UP OK!") == 0)
  {
   cout << "登录成功。" << endl;
   g_bAuth = true;
  }
  else if(strcmp(szBuf, "U Err!") == 0)
  {
   cout << "此用户不存在。" << endl;
   g_bAuth = false;
  }
  else if(strcmp(szBuf, "P Err!") == 0)
  {
   cout << "密码错误。" << endl;
   g_bAuth = false;
  }
 }
 return g_bAuth;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : GetFile
// 功能描述     : 取得服务器的文件
// 参数         : SOCKET sock
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void GetFile(SOCKET sock)
{
 if(!g_bAuth)
 {
  OutMsg("用户还没登录！请先登录");
  return;
 }
 
 char szSrcFile[MAX_PATH];
 char szDstFile[MAX_PATH];
 memset(szSrcFile, 0, MAX_PATH);
 memset(szDstFile, 0, MAX_PATH);

 cout << "你要取得Server上的文件：";
 cin >> szSrcFile;

 cout << "你要把文件存在哪里：";
 cin >> szDstFile;

 SCommand cmd;
 cmd.CommandID = CMD_GETFILE;
 cmd.DataSize = strlen(szSrcFile);

 // 发送命令
 SendFix(sock, (char *)&cmd, sizeof(cmd));
 
 // 发送文件名
 SendFix(sock, szSrcFile, strlen(szSrcFile));

 // 接收文件长度
 DWORD dwFileSize = 0;
 RecvFix(sock, (char*)&dwFileSize, sizeof(dwFileSize));
 
 if(dwFileSize == 0)
 {
  OutMsg("文件不存在");
  return;
 }

 // 接收文件内容
 DWORD dwLeft = dwFileSize;
 char szBuf[1024];
 HANDLE hFile = CreateFile(szDstFile, GENERIC_WRITE, FILE_SHARE_READ, 
  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
 if(hFile == INVALID_HANDLE_VALUE)
 {
  hFile = CreateFile(szDstFile, GENERIC_WRITE, FILE_SHARE_READ, 
   NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hFile == INVALID_HANDLE_VALUE)
  {
   OutErr("CreateFile failed!");
   return;
  }
 }
 while(dwLeft > 0)
 {
  memset(szBuf, 0, 1024);
  // 这里是不确定接收长度的，所以要用recv，不能用RecvFix
  int nRead = recv(sock, szBuf, 1024, 0);
  if(nRead == SOCKET_ERROR)
   OutErr("RecvFix Error!");

  DWORD dwWritten = 0;
  if(!WriteFile(hFile, szBuf, nRead, &dwWritten, NULL))
  {
   OutErr("WriteFile error!");
   return;
  }
  dwLeft -= dwWritten;
 }

 CloseHandle(hFile);

 OutMsg("接收文件成功！");
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : RegisterUser
// 功能描述     : 注册新用户
// 参数         : SOCKET sock
// 参数         : char *szName
// 参数         : char *szPwd
// 返回值       : bool 
//
///////////////////////////////////////////////////////////////////////
bool RegisterUser(SOCKET sock, char *szName, char *szPwd)
{
 char szCmd[50];
 memset(szCmd, 0, 50);
 strcpy(szCmd, szName);
 strcat(szCmd, " ");
 strcat(szCmd, szPwd);
 
 SCommand cmd;
 cmd.CommandID = CMD_REGISTER;
 cmd.DataSize = strlen(szCmd);

 // 发送命令
 int nRet = SendFix(sock, (char *)&cmd, sizeof(cmd));
 if(nRet == SOCKET_ERROR)
 {
  OutErr("SendFix() failed!");
  return false;
 }
 else
 {
  // 发送用户名和密码串
  SendFix(sock, szCmd, strlen(szCmd));
  char szBuf[10];
  memset(szBuf, 0, 10);
  
  recv(sock, szBuf, 10, 0);
  if(strcmp(szBuf, "REG OK!") == 0)
  {
   cout << "注册成功。" << endl;
   return true;
  }
  else if(strcmp(szBuf, "REG ERR!") == 0)
  {
   cout << "注册失败." << endl;
   return false;
  }
 }
 
 return false;
}