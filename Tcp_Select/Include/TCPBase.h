#pragma once
#include <Winsock2.h>
#define OutErr(a) 

class CTcpBase
{
public:
	CTcpBase(void)
	{
		m_tv.tv_usec=0;
		m_tv.tv_sec=5;//延迟5秒;
	}
	~CTcpBase(void)
	{

	}
private:
	fd_set m_fdwrite;
	timeval m_tv;
	fd_set m_fdread;
public:
	// 发送
	long Sendn(SOCKET mSocket,char* Pkg,int nSize)
	{
		//检查网络是否可写
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
					return -1;//网路断开
				}
			}
			Pos +=nLen;
		}
		return Pos;//返回发送长度
	}
	// 接收
	long Recvn(SOCKET mSocket,char* Pkg,int nMaxSize)
	{
		//接收信息长度头(INT)型
		int msgRealLen=0;
		int Pos=0;
		int  nSize=0;
		//检查网络可否读写
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
			if (nSize<0)//接收数据出现错误
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
		//接收的信息包长度
		msgRealLen=*(int*)Pkg;       
		msgRealLen=ntohl(msgRealLen);//网络字节转换
		if (msgRealLen<=0||msgRealLen>nMaxSize-4)
		{
			//(_T("接收信息长度错误."));
			return -1;
		}
		Pos=0;
		while(Pos < msgRealLen)
		{
			nSize=recv(mSocket,(char*)Pkg+ Pos,msgRealLen-Pos,0);
			if (nSize<=0)//接收数据出现错误
			{
				if (nSize<0)//接收数据出现错误
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
		return msgRealLen;//返回实际接收的数据的长度
	}
};
