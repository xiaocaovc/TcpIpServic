
// ClientTestDlg.h : 头文件
//

#pragma once
#include "TcpClient.h"

// CClientTestDlg 对话框
class CClientTestDlg : public CDialog
{
// 构造
public:
	CClientTestDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CLIENTTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtConn();
	afx_msg void OnBnClickedBtSend();
	afx_msg void OnBnClickedBtRecv();
	CTcpClient* m_pClient;
	afx_msg void OnBnClickedBtStop();
	afx_msg void OnClose();
};
