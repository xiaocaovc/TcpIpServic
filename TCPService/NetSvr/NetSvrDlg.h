
// NetSvrDlg.h : ͷ�ļ�
//

#pragma once
#include"TcpService.h"
#include <winsock2.h>
#include <MSWSock.h>
#include "NetServiceExp.h"

// CNetSvrDlg �Ի���
class CNetSvrDlg : public CDialogEx
{
// ����
public:
	CNetSvrDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NETSVR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
