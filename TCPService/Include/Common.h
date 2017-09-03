#ifndef _COMMON_STRUCT_IO_
#define _COMMON_STRUCT_IO_

typedef enum net_operation_s
{
	NULL_POSTED,    // ���ڳ�ʼ����������
	ACCEPT_POSTED, // ��־Ͷ�ݵ�Accept����
	SEND_POSTED,   // ��־Ͷ�ݵ��Ƿ��Ͳ���
	RECV_POSTED,   // ��־Ͷ�ݵ��ǽ��ղ���
	WRITE_POSTED,  // ��־Ͷ�ݵ���д�����
	READ_POSTED,   // ��־Ͷ�ݵ��Ƕ�ȡ����

}net_operation_t;
typedef struct net_completionkeyex_s
{
	SOCKET s;
	int   active; // 0,1
}net_completionkeyex_t;
typedef struct _overlappedex_
{
	OVERLAPPED      m_Overlapped; // �ص��ṹ
	OPERATION_TYPE  m_OpType;     // ������ʶ
	DWORD			m_dwFlags;
	DWORD			m_dwBytes;
	WSABUF			m_wsaBuf[1];
}
#endif