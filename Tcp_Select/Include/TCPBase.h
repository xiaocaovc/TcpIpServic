#pragma once
#include <Winsock2.h>
#define OutErr(a) 

class CTcpBase
{
public:
	CTcpBase(void)
	{
		m_tv.tv_usec=0;
		m_tv.tv_sec=5;//�ӳ�5��;
	}
	~CTcpBase(void)
	{

	}
private:
	fd_set m_fdwrite;
	timeval m_tv;
	fd_set m_fdread;
public:
	// ����
	long Sendn(SOCKET mSocket,char* Pkg,int nSize)
	{
		//��������Ƿ��д
		FD_ZERO(&fdwrite);
		FD_SET(mSocket,&fdwrite);
		switch (select(0,NULL,&m_fdwrite,NULL,&m_tv))
		{
		case -1:// error handled by u;
			OutErr("Sendn select fail!");
			return -1;
		case 0: // timeout hanled by u;
			OutErr("Sendn select timeout!");
			return 0;
		default:
			if (!FD_ISSET(mSocket,&m_fdwrite)) 
			{
				return 0;
			} 
		} 
		int Pos=0;
		int nLen=0;
		while(Pos<nSize)
		{
			nLen = send(mSocket,Pkg+Pos , nSize-Pos, 0);
			if(nLen <= 0)
			{
				if(errno == EINTR || errno == WSAEWOULDBLOCK || errno == EAGAIN)
				{
					Sleep(1);
					continue;
				}
				else
				{
					return -1;//��·�Ͽ�
				}
			}
			Pos +=nLen;
		}
		return Pos;//���ط��ͳ���
	}
	// ����
	long Recvn(SOCKET mSocket,char* Pkg,int nMaxSize)
	{
		//������Ϣ����ͷ(INT)��
		int msgRealLen=0;
		int Pos=0;
		int  nSize=0;
		//�������ɷ��д
		FD_ZERO(&m_fdread);
		FD_SET(mSocket,&m_fdread);
		switch (select(0,&m_fdread,NULL,NULL,&m_tv))
		{
		case -1:
			return -1;
			//error handled by u; 
		case 0:
			return 0;
			//timeout hanled by u; 
		default:
			if (!FD_ISSET(mSocket,&fdread)) 
			{
				return -1;
			} 
		} 
		while(Pos < sizeof(int))
		{
			nSize=recv(mSocket,Pkg + Pos,sizeof(int)-Pos,0);
			if (nSize<0)//�������ݳ��ִ���
			{
				Sleep(10);
				continue;
			}
			if (nSize==0)
			{
				return -1;
			}
			Pos +=nSize;
		}
		//���յ���Ϣ������
		msgRealLen=*(int*)Pkg;       
		msgRealLen=ntohl(msgRealLen);//�����ֽ�ת��
		if (msgRealLen<=0||msgRealLen>nMaxSize-4)
		{
			//(_T("������Ϣ���ȴ���."));
			return -1;
		}
		Pos=0;
		while(Pos < msgRealLen)
		{
			nSize=recv(mSocket,(char*)Pkg+ Pos,msgRealLen-Pos,0);
			if (nSize<=0)//�������ݳ��ִ���
			{
				if (nSize<0)//�������ݳ��ִ���
				{
					Sleep(10);
					continue;
				}
				if (nSize==0)
				{
					return -1;
				}
			}
			Pos+=nSize;
		}
		return msgRealLen;//����ʵ�ʽ��յ����ݵĳ���
	}
};
