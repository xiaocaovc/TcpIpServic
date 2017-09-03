/*/
�ļ���SocketClient.cpp

˵����
 ���ļ�����Ϊ���ԵĿͻ��ˣ�ʵ���˵�¼��ȡ�ļ��Ĺ��ܡ�
 �ͷ���˵Ľ������ǲ����˷���������ݳ��ȣ�Ȼ���;��������������˳��
 ��ϸ�ɿ�����˵�˵����

 �����߼��������ģ��ͻ���Ҫ�ȵ�¼����ˣ�Ȼ���¼�ɹ�֮�󣬲��ܽ�����Ӧ�Ĳ�����

 ���������ò�̫�ã��Ժ��ٲ����ˡ�

 ��������ע�ͣ��ṹ�������ȵı���淶�����˸��˱Ƚ�ϲ���ķ�ʽ��

�����
 ../Bin/SocketClient.exe

�÷���
 ���� SocketClient Server_IP
 ����ֱ������SocketClient������ʾ���������˵�IP

Todo:
 ��һ��������ɸ���SOCKET��ģ�ͣ�Ȼ�󹫿��Լ����о����롣
 ���ܷ�����ǣ�
 1������������ָ�������ļ���
 2���ͻ��˿����г���������������Щ�ļ�
 3���ͻ��˿����г���Щ�û����ߣ������Է�����������û�����
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
// ������       : Usage
// ��������     : ��ʾ�����÷�
// ����ֵ       : void 
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
// ������       : Menu
// ��������     : ѡ�����Ľ���
// ����ֵ       : void 
//
///////////////////////////////////////////////////////////////////////
void Menu()
{
 system("cls");
 printf("********************************************/n"); 
 printf("��ѡ�������        /n/n"); 
 printf("1��ȡ���ļ�         /n"); 
 printf("2���˳�          /n"); 
 printf("********************************************/n"); 
}


///////////////////////////////////////////////////////////////////////
//
// ������       : LoginMenu
// ��������     : �û���¼�Ľ���
// ����ֵ       : void 
//
///////////////////////////////////////////////////////////////////////
void LoginMenu()
{
 cout << "�밴���������������" <<endl;
 getchar();
 system("cls");
 printf("********************************************/n"); 
 printf("��ѡ�������        /n/n"); 
 printf("1����¼          /n"); 
 printf("2��ע��          /n"); 
 printf("3���˳�          /n"); 
 printf("********************************************/n"); 
}


///////////////////////////////////////////////////////////////////////
//
// ������       : Login
// ��������     : �û���¼�Ľ����߼�
// ����         : SOCKET sock
// ����ֵ       : bool 
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
    cout << "����������û�����";
    cin >> szName;
    cout << "������������룺";
    cin >> szPwd;
    if(Auth(sock, szName, szPwd))
    {
     return true; 
    }
    else
    {
     char c;
     cout << "������¼��y/n" << endl;
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
   cout << "����������û�����";
   cin >> szName;
   cout << "������������룺";
   cin >> szPwd;
   cout << "���ٴ�����������룺";
   cin >> szConfirmPwd;
   if(strcmp(szPwd, szConfirmPwd) != 0)
   {
    cout << "ǰ�����벻һ��" << endl;
   }
   else
   {
    if(!RegisterUser(sock, szName, szPwd))
    {
     cout << "ע���û�ʧ��!" << endl;
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
  cout << "�����������IP��";
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
  OutErr("���ӷ�����ʧ�ܣ�");
  return;
 }
 else
 {
  OutMsg("�Ѻͷ������������ӣ�");
 }

 // Ҫ���û���¼
 if(!Login(sockServer))
  return;

 // ��¼�ɹ������û�ѡ�����
 int nChoose = 0;
 bool bExit = false;
 while(!bExit)
 {
  Menu();
  cin >> nChoose;
  switch(nChoose)
  {
  case 1:  // ��ȡ�ļ�
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
// ������       : Auth
// ��������     : �û���¼��֤
// ����         : SOCKET sock
// ����         : char *szName
// ����         : char *szPwd
// ����ֵ       : bool 
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
   cout << "��¼�ɹ���" << endl;
   g_bAuth = true;
  }
  else if(strcmp(szBuf, "U Err!") == 0)
  {
   cout << "���û������ڡ�" << endl;
   g_bAuth = false;
  }
  else if(strcmp(szBuf, "P Err!") == 0)
  {
   cout << "�������" << endl;
   g_bAuth = false;
  }
 }
 return g_bAuth;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : GetFile
// ��������     : ȡ�÷��������ļ�
// ����         : SOCKET sock
// ����ֵ       : void 
//
///////////////////////////////////////////////////////////////////////
void GetFile(SOCKET sock)
{
 if(!g_bAuth)
 {
  OutMsg("�û���û��¼�����ȵ�¼");
  return;
 }
 
 char szSrcFile[MAX_PATH];
 char szDstFile[MAX_PATH];
 memset(szSrcFile, 0, MAX_PATH);
 memset(szDstFile, 0, MAX_PATH);

 cout << "��Ҫȡ��Server�ϵ��ļ���";
 cin >> szSrcFile;

 cout << "��Ҫ���ļ��������";
 cin >> szDstFile;

 SCommand cmd;
 cmd.CommandID = CMD_GETFILE;
 cmd.DataSize = strlen(szSrcFile);

 // ��������
 SendFix(sock, (char *)&cmd, sizeof(cmd));
 
 // �����ļ���
 SendFix(sock, szSrcFile, strlen(szSrcFile));

 // �����ļ�����
 DWORD dwFileSize = 0;
 RecvFix(sock, (char*)&dwFileSize, sizeof(dwFileSize));
 
 if(dwFileSize == 0)
 {
  OutMsg("�ļ�������");
  return;
 }

 // �����ļ�����
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
  // �����ǲ�ȷ�����ճ��ȵģ�����Ҫ��recv��������RecvFix
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

 OutMsg("�����ļ��ɹ���");
}


///////////////////////////////////////////////////////////////////////
//
// ������       : RegisterUser
// ��������     : ע�����û�
// ����         : SOCKET sock
// ����         : char *szName
// ����         : char *szPwd
// ����ֵ       : bool 
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

 // ��������
 int nRet = SendFix(sock, (char *)&cmd, sizeof(cmd));
 if(nRet == SOCKET_ERROR)
 {
  OutErr("SendFix() failed!");
  return false;
 }
 else
 {
  // �����û��������봮
  SendFix(sock, szCmd, strlen(szCmd));
  char szBuf[10];
  memset(szBuf, 0, 10);
  
  recv(sock, szBuf, 10, 0);
  if(strcmp(szBuf, "REG OK!") == 0)
  {
   cout << "ע��ɹ���" << endl;
   return true;
  }
  else if(strcmp(szBuf, "REG ERR!") == 0)
  {
   cout << "ע��ʧ��." << endl;
   return false;
  }
 }
 
 return false;
}