/*/
�ļ�: CommonCmd.h
˵��: 
 ʵ���˷���˺Ϳͻ���һЩ���õ����ݽṹ�����Է���˺Ϳͻ��˶�Ҫ������
 ���������SOCKET�ĵ�ǰ״̬�ȵĶ��塣
/*/

#ifndef __COMMONCMD_H__
#define __COMMONCMD_H__

#define PORT 5050

// �����
#define CMD_AUTHEN 1 // ��¼��֤
#define CMD_GETFILE 2 // ��ȡ�ļ�
#define CMD_REGISTER 3  // ע���û�

typedef struct tagCommand
{
 int CommandID;  // ����ID
 DWORD DataSize;  // ������ݵĴ�С
}SCommand;

// ��־Ŀǰ��SOCKET����ʲô
enum ECurOp
{RecvCmd, RecvData, ExecCmd};


#endif //__COMMONCMD_H__