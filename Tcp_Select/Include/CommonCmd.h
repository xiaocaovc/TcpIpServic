/*/
文件: CommonCmd.h
说明: 
 实现了服务端和客户端一些公用的数据结构，所以服务端和客户端都要包含。
 其中有命令、SOCKET的当前状态等的定义。
/*/

#ifndef __COMMONCMD_H__
#define __COMMONCMD_H__

#define PORT 5050

// 命令定义
#define CMD_AUTHEN 1 // 登录认证
#define CMD_GETFILE 2 // 获取文件
#define CMD_REGISTER 3  // 注册用户

typedef struct tagCommand
{
 int CommandID;  // 命令ID
 DWORD DataSize;  // 后接数据的大小
}SCommand;

// 标志目前的SOCKET该做什么
enum ECurOp
{RecvCmd, RecvData, ExecCmd};


#endif //__COMMONCMD_H__