
// NetSvrDlg.h : 头文件
//

#pragma once
#include"TcpService.h"
#include <winsock2.h>
#include <MSWSock.h>
#include "NetServiceExp.h"

// CNetSvrDlg 对话框
class CNetSvrDlg : public CDialogEx
{
// 构造
public:
	CNetSvrDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NETSVR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtStart();
	CTcpService m_Svr;
	SOCKET m_Socket;
	LPFN_ACCEPTEX m_lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS m_lpfnGetAcceptExSockAddrs;
	OVERLAPPED   m_Overlapped;
};
