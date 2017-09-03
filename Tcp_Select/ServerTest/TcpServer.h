#pragma once
#include <winsock2.h>
#pragma comment(lib, "WS2_32")
#define OutErr(a) 

class CTcpServer
{
public:
	CTcpServer(void)
	:m_nSocket(INVALID_SOCKET)
	{

	}
	~CTcpServer(void)
	{
		if (INVALID_SOCKET!=m_nSocket)
		{
			closesocket(m_nSocket);
			m_nSocket=INVALID_SOCKET;
		}
	}
private:
	// ����SOCKET
	SOCKET m_nSocket;
	FD_SET m_fdRead;
	// ��ʱ����
	timeval m_tv;
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
	// ����
	static long Sendn(SOCKET nSocket,char* Pkg,int nSize)
	{
		fd_set fdwrite;
		timeval tv;
		tv.tv_usec=0;
		tv.tv_sec=5;//5��;
		while(true)
		{
			//��������Ƿ��д
			FD_ZERO(&fdwrite);
			FD_SET(nSocket,&fdwrite);
			switch (select(0,NULL,&fdwrite,NULL,&tv))
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
				if (!FD_ISSET(nSocket,&fdwrite)) 
				{
					// Sleep(1);
					continue;
				} 
				break;
			}
			break;
		}
		// ��������
		int nPos=0,nLen=0;
		while(nPos<nSize)
		{
			nLen = send(nSocket,Pkg+nPos , nSize-nPos, 0);
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
	// ����
	static long Recvn(SOCKET nSocket,char* Pkg,int nSize)
	{
		timeval tv;
		tv.tv_usec=0;
		tv.tv_sec=5;//5��;
		fd_set fdread;
		while(true)
		{
			//�������ɷ��д
			FD_ZERO(&fdread);
			FD_SET(nSocket,&fdread);
			switch (select(0,&fdread,NULL,NULL,&tv))
			{
			case -1:
				{
					return -1;
					//error handled by u; 
				}
			case 0:
				{
					// Sleep(1);
					continue;
				} 
			default:
				if (!FD_ISSET(nSocket,&fdread)) 
				{
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
			nLen=recv(nSocket,(char*)Pkg+nPos,nSize-nPos,0);
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
	// ����������,���ɼ���SOCKET
	bool CreateServer(unsigned short SerPort)
	{
		m_tv.tv_usec=0;
		m_tv.tv_sec=10;//10��;
		// ��������socket 
		m_nSocket = socket(AF_INET, SOCK_STREAM, 0);

		// �󶨶˿�
		struct sockaddr_in servAddr;
		servAddr.sin_family = AF_INET;
		servAddr.sin_port = htons(SerPort);
		servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		if(bind(m_nSocket, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
		{
			OutErr("bind Failed!");
			closesocket(m_nSocket);
			m_nSocket=INVALID_SOCKET;
			WSACleanup();
			return false;
		}
		// ���ü�������Ϊ200
		if(listen(m_nSocket, 200) != 0)
		{
			closesocket(m_nSocket);
			m_nSocket=INVALID_SOCKET;
			WSACleanup();
			OutErr("listen Failed!");
			return false;
		}
		// ���ü���Ϊ������ģʽ
		ULONG NonBlock = 1;
		ioctlsocket(m_nSocket, FIONBIO, &NonBlock);
		return true;
	}
	// �����ͻ�������
	SOCKET ListenConnect(char* szClientIp, unsigned short* uClientPort)
	{
		FD_ZERO(&m_fdRead);
		// ���úö���
		FD_SET(m_nSocket,&m_fdRead);
		switch (select(0,&m_fdRead,NULL,NULL,&m_tv))
		{
		case -1:// error handled by u; 
		 {
			 OutErr("select() Failed!");
			 return INVALID_SOCKET;
		 }
		case 0: // timeout hanled by u; 
			return 0;
		default:
			// ˵�����Խ���������
			if (!FD_ISSET(m_nSocket,&m_fdRead)) 
		 {
			 return 0;
			} 
		}
		SOCKADDR_IN addrClient;
		int len=sizeof(SOCKADDR);
		SOCKET nSocket=accept(m_nSocket,(SOCKADDR*)&addrClient,&len);
		if (nSocket== INVALID_SOCKET) //SOCKET_ERROR
		{
			return 0;
		}
		strcpy(szClientIp,inet_ntoa(addrClient.sin_addr));
		*uClientPort=addrClient.sin_port;
		unsigned long iMode = 1; //0���� ��1 ������
		ioctlsocket(nSocket,FIONBIO,&iMode);//����������socket��Ϊ�첽ģʽ
		return nSocket;
	}
	bool StopServer()
	{
		if (INVALID_SOCKET!=m_nSocket)
		{
			closesocket(m_nSocket);
			m_nSocket=INVALID_SOCKET;
		}
		return true;
	}
};
