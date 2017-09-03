#pragma once
#include <winsock2.h>
#pragma comment(lib, "WS2_32")
#define OutErr(a) 

class CTcpClient
{
public:
	CTcpClient(void)
	:m_nSocket(INVALID_SOCKET)
	{
	}
	~CTcpClient(void)
	{
		if (INVALID_SOCKET!=m_nSocket)
		{
			closesocket(m_nSocket);
			WSACleanup();
		}
	}
private:
	SOCKET m_nSocket;
	fd_set m_fdwrite;
	timeval m_tv;
	fd_set m_fdread;
public:
	// ��ʼ��WinSocket
	static bool InitWinSocket()
	{
		// ��ʼ��WINSOCK
		WSADATA wsd;
		if(WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
		{
			OutErr("WSAStartup()");
			return false;
		}
		return true;
	}
	// �ر�WinSocket
	static void CloseWinSocket()
	{
		WSACleanup();
	}
public:
	// ���� ����ֵ-1�������(�Ͽ�),�������͵�����
	long Sendn(char* Pkg,const int nSize)
	{
		while(true)
		{
			//��������Ƿ��д
			FD_ZERO(&m_fdwrite);
			FD_SET(m_nSocket,&m_fdwrite);
			switch (select(0,NULL,&m_fdwrite,NULL,&m_tv))
			{
			case -1:// error handled by u;
				{
					return -1;
				}
			case 0: // timeout hanled by u;
				{
					// Sleep(1);
					continue;
				}
			default:
				if (!FD_ISSET(m_nSocket,&m_fdwrite))
				{
					// Sleep(1);
					continue;
				} 
				break;
			}
			break;
		}
			// ��������
		int nPos=0;
		int nLen=0;
		while(nPos<nSize)
		{
			nLen = send(m_nSocket,Pkg+nPos , nSize-nPos, 0);
			if(SOCKET_ERROR==nLen||0==nLen)
			{
				if(WSAEWOULDBLOCK == WSAGetLastError())
				{
					// Sleep(1);
					continue;
				}
				else
				{
					return -1;//��·�Ͽ�
				}
			}
			nPos +=nLen;
		}
		return nPos;//���ط��ͳ���
	}
	// ����,����ֵ-1,�������(�Ͽ�),�������յ�������
	long Recvn(char* Pkg,const int nSize)
	{
		while(true)
		{
			//�������ɷ��д
			FD_ZERO(&m_fdread);
			FD_SET(m_nSocket,&m_fdread);
			switch (select(0,&m_fdread,NULL,NULL,&m_tv))
			{
			case -1: //��·�Ͽ�
				{
					return -1;
					//error handled by u;
				} 
			case 0: //timeout hanled by u;
				{
					// Sleep(1);
					continue;
				} 
			default:
				if (!FD_ISSET(m_nSocket,&m_fdread)) 
				{
					// Sleep(1);
					continue;
				} 
				break;
			}
			break;
		}
		int nPos=0,nLen=0;
		while(nPos<nSize)
		{
			// ��������
			nLen=recv(m_nSocket,(char*)Pkg+nPos,nSize-nPos,0);
			if(SOCKET_ERROR==nLen||0==nLen)
			{
				if(WSAEWOULDBLOCK == WSAGetLastError())
				{
					// Sleep(1);
					continue;
				}
				else
				{
					return -1;//��·�Ͽ�
				}
			}
			nPos+=nLen;
		}
		return nPos;//����ʵ�ʽ��յ����ݵĳ���
	}
	// ���ӷ�����
	bool ConnectServer(char* szSerIP, unsigned short nSerPort)
	{
		m_nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET==m_nSocket)
		{
			return false;
		}
		struct hostent *pHost = NULL;
		struct sockaddr_in servAddr;
		servAddr.sin_family = AF_INET;
		servAddr.sin_port = htons(nSerPort);
		servAddr.sin_addr.s_addr = inet_addr(szSerIP);
		// ������������������ֶ�����IP��ַ
		if(servAddr.sin_addr.s_addr == INADDR_NONE)
		{
			pHost = gethostbyname(szSerIP);
			if(pHost == NULL)
			{
				closesocket(m_nSocket);
				WSACleanup();
				m_nSocket=INVALID_SOCKET;
				OutErr("gethostbyname Failed!");
				return false;
			}
			memcpy(&servAddr.sin_addr, pHost->h_addr_list[0], pHost->h_length);
		}

		int nRet = 0;
		nRet=connect(m_nSocket, (struct sockaddr*)&servAddr, sizeof(servAddr));
		if(SOCKET_ERROR==nRet)
		{
			closesocket(m_nSocket);
			WSACleanup();
			m_nSocket=INVALID_SOCKET;
			OutErr("connect failed!");
			return false;
		}
		// ����Ϊ������ģʽ
		ULONG NonBlock = 1;
		ioctlsocket(m_nSocket, FIONBIO, &NonBlock);
		m_tv.tv_usec=0;
		m_tv.tv_sec=5;//10��;
		return true;
	}
	bool StopClient()
	{
		if (INVALID_SOCKET!=m_nSocket)
		{
			closesocket(m_nSocket);
			m_nSocket=INVALID_SOCKET;
		}
		return true;
	}
};
