/*/
�ļ���SelectServer.cpp
˵����

 ���ļ���ʾ�����ʹ��selectģ�����������������ѵ���select��writefds��ʲôʱ��ʹ�á�
 �úÿ���������ܺ����׵��ˣ�����˵��д��Щ�������Ϊ��̽���������ģ����˺ܶ����϶��Ҳ�������

 �������һ����Ƿ����ͬʱ��дͬһ��SOCKET����������ǿ��Եģ�������ñ�����������Ϊ�ᵼ�°���˳��һ�¡�

    ����˵һ��SELECTģ�͵��߼���
 ���������ʹ��selectģ�ͣ��ڵ���recv����sendʱ��ᵼ�³������������ʹ����select
 �͸�����������һ�㱣��������˵�ڵ�����select����֮�󣬶Դ��ڶ����ϵ�socket����recv����
 ��һ����ɹ��ģ����ǲ���ϵͳ�����Ǳ�֤�ģ��������ж�SOCKET�Ƿ��дʱҲһ����
 ���Ҿ��㲻�ɶ��򲻿�д��ʹ����selectҲ������ ������Ϊ select �����ṩ�˳�ʱ������������Ի�����
 ���첽connect��Ҳ���ǿ���ɨ�����������ĸ��������˷���Զ�̿����������������Ŷ����

 ����������������߼���������ǵ�server�أ�
 �����õķ����ǽ���һ��SocketInfo�����SocketInfo�����˶�Socket��ǰ���еĲ������Ұ�����Ϊ��
 {RecvCmd, RecvData, ExecCmd} һ��ʼsocket�Ǵ���һ��RecvCmd��״̬��
 Ȼ��ȡ����CMD��Ҳ����ȡ����ָ�������һ��CPU�õ���ָ����ʲô����Ȼ���Ҫȡ�����ˣ�ȡ��ָ��
 ֪��Ҫ��ʲô��ȡ�������ݾͿ���ʵ�ʿ�ʼ���ˡ�ʵ�ʿ�ʼ�ɾ���ExecCmd�������״̬֮������Ҫ
 �������ݵ��ˣ����԰����Ƕ������ж�SOCKET��д����<���� if(FD_ISSET(vecSocketInfo[i].sock, &fdWrite)) >��
 ����Socket��д�Ϳ��Է�����Ϣ���ͻ����ˡ�

 ���͵ĸ���Э���������ģ��ȷ�һ��SCommand�Ľṹ���ȥ������ṹ��˵����ָ������ݵĳ��ȡ�
 Ȼ��͸���������Ƚ������ݡ�����ٸ��ͻ���������Ӧ����Ӧ��

    �������ִ���ṹ�����Ժܷ��������µĹ��ܡ�

   ���������ò�̫�ã��Ժ��ٲ����ˡ�

 ��������ע�ͣ��ṹ�������ȵı���淶�����˸��˱Ƚ�ϲ���ķ�ʽ��

�����
 ../Bin/SelectServer.exe

�÷���
 ֱ�������Ϳ�����

Todo:
 ��һ��������ɸ���SOCKET��ģ�ͣ�Ȼ�󹫿��Լ����о����롣
 ���ܷ�����ǣ�
 1������������ָ�������ļ���
 2���ͻ��˿����г���������������Щ�ļ�
 3���ͻ��˿����г���Щ�û����ߣ������Է�����������û�����
 4�����Ͻ���
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

// ��¼�û����б�
map<string, SOCKET> g_LoginUsers;

// ע���û����б�(�û���������)
map<string, string> g_RegUSers;

// �����˳�������
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
// ������       : RemoveByIndex
// ��������     : ���� index ��ɾ�� VECTOR ���Ԫ��
// ����         : vector<T> &vec [in]
// ����         : int nIdx   [in]
// ����ֵ       : void 
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
  // ÿ�ε���select֮ǰ��Ҫ�Ѷ�����д�����
  FD_ZERO(&fdRead);
  FD_ZERO(&fdWrite);
  
  // ���úö�����д��
  FD_SET(sockListen, &fdRead);
  for(int i = 0; i < vecSocketInfo.size(); i++)
  {
   FD_SET(vecSocketInfo[i].sock, &fdRead);
   FD_SET(vecSocketInfo[i].sock, &fdWrite);
  }

  // ����select����
  if(select(0, &fdRead, &fdWrite, NULL, NULL) == SOCKET_ERROR)
  {
   OutErr("select() Failed!");
   break;
  }

  // ˵�����Խ���������
  if(FD_ISSET(sockListen, &fdRead))
  {
   char szClientIP[50];
   sockClient = AcceptClient(sockListen, szClientIP);
   cout << szClientIP << " ��������" << endl;

   ioctlsocket(sockClient, FIONBIO, &NonBlock);

   SSocketInfo sockInfo;
   sockInfo.sock = sockClient;
   sockInfo.eCurOp = RecvCmd;
   // �ѽ��յ������socket�����Լ��Ķ�����
   vecSocketInfo.push_back(sockInfo);
  }

  for(int i = 0; i < vecSocketInfo.size(); i++)
  {
   // ����ɶ�
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

   // �����д
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
// ������       : DoRecvCmd
// ��������     : ��ȡ�ͻ��˴�������cmd
// ����         : vector<SSocketInfo> &vecSockInfo
// ����         : int idx
// ����ֵ       : void 
//
///////////////////////////////////////////////////////////////////////
void DoRecvCmd(vector<SSocketInfo> &vecSockInfo, int idx)
{
 SSocketInfo *sockInfo = &vecSockInfo[idx];
 int nRet = RecvFix(sockInfo->sock, (char *)&(sockInfo->cmd), sizeof(sockInfo->cmd));

 // ����û�������¼�������� closesocket �ر� socket �᷵��0 
 // ����û�ֱ�ӹرճ���᷵�� SOCKET_ERROR��ǿ�йر�
 if(nRet == SOCKET_ERROR || nRet == 0)
 {
  OutMsg("�ͻ������˳���");
  closesocket(sockInfo->sock);
  sockInfo->sock = INVALID_SOCKET;     
  EraseByIndex(vecSockInfo, idx);
  return;
 }
 sockInfo->eCurOp = RecvData;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : DoRecvData
// ��������     : DoRecvCmd �Ѿ������ָ���������Ҫ���ִ��ָ������Ҫ������
// ����         : vector<SSocketInfo> &vecSockInfo
// ����         : int idx
// ����ֵ       : void 
//
///////////////////////////////////////////////////////////////////////
void DoRecvData(vector<SSocketInfo> &vecSockInfo, int idx)
{
 SSocketInfo *sockInfo = &vecSockInfo[idx];
 // Ϊ���ݷ���ռ䣬�����һλ����������0
 sockInfo->data = new char[sockInfo->cmd.DataSize + 1];
 memset(sockInfo->data, 0, sockInfo->cmd.DataSize + 1);
 
 // ��������
 int nRet = RecvFix(sockInfo->sock, sockInfo->data, sockInfo->cmd.DataSize);
 if(nRet == SOCKET_ERROR || nRet == 0)
 {
  OutMsg("�ͻ������˳���");
  closesocket(sockInfo->sock);
  sockInfo->sock = INVALID_SOCKET;     
  EraseByIndex(vecSockInfo, idx);
  return;
 }
  
 sockInfo->eCurOp = ExecCmd;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : DoExecCmd
// ��������     : ָ���ִ��ָ���������ݶ��Ѿ�׼�����ˣ��������Ϳ���ִ������
// ����         : vector<SSocketInfo> &vecSockInfo
// ����         : int idx
// ����ֵ       : void 
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

 // ִ�������������ûؽ���ָ��״̬
 sockInfo->eCurOp = RecvCmd;
}

///////////////////////////////////////////////////////////////////////
//
// ������       : DoAuthen
// ��������     : ���û�������������֤
// ����         : SOCKET sock
// ����         : char *data
// ����         : DWORD len
// ����ֵ       : bool 
//
///////////////////////////////////////////////////////////////////////
bool DoAuthen(SOCKET sock, char *data, DWORD len)
{
 // ȡ���û�����������ַ���
 // ��ʽΪ "dyl 123"

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
 // ����������û�
  strcpy(szSend, "U Err!");
 }
 
 int nRet = SendFix(sock, szSend, strlen(szSend));

 if(nRet == SOCKET_ERROR)
  return false;

 // ִ�����˾��ͷ�data
 delete []data;

 return true;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : DoGetFile
// ��������     : Ϊ�û��ṩ�ļ�
// ����         : SOCKET sock
// ����         : char *data
// ����         : DWORD len
// ����ֵ       : bool 
//
///////////////////////////////////////////////////////////////////////
bool DoGetFile(SOCKET sock, char *data, DWORD len)
{
 // ���ļ����ж��ļ��Ƿ����
 HANDLE hFile = CreateFile(data, GENERIC_READ, FILE_SHARE_READ, 
  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
 
 if(hFile == INVALID_HANDLE_VALUE)
 {
  OutMsg("�ļ������ڣ�");
  DWORD dwSize = 0;
  SendFix(sock, (char *)&dwSize, sizeof(dwSize));
  return false;
 }
 else
 {// �����ļ���Ϣ

  // �����ļ���С�����͹�ȥ
  DWORD dwFileSize = GetFileSize(hFile, NULL);
  int nRet = SendFix(sock, (char *)&dwFileSize, sizeof(dwFileSize));
  if(nRet == SOCKET_ERROR)
   return false;
  
  // ���ļ���¼������
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
 // ȡ���û�����������ַ���
 // ��ʽΪ "dyl 123"

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
   OutMsg("�����ļ�Users.lstʧ�ܣ�");
   strcpy(szSend, "REG ERR!");
   bReturn = false;
  }
  else
  {
   // �ڿ�ʼ��
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

   // ���ص���ע���û��б���
   GetRegUsers();

   strcpy(szSend, "REG OK!");
  }
 }
 else
 {
  // �ƶ������׷��
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

  // ���ص���ע���û��б���
  GetRegUsers();

  strcpy(szSend, "REG OK!");  
 }
 int nRet = SendFix(sock, szSend, strlen(szSend));
 if(nRet == SOCKET_ERROR)
  bReturn = false;

 // ִ�����˾��ͷ�data
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
  OutMsg("�û��б����ڣ�");
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