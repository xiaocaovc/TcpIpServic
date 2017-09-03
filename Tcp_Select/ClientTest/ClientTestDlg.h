
// ClientTestDlg.h : ͷ�ļ�
//

#pragma once
#include "TcpClient.h"

// CClientTestDlg �Ի���
class CClientTestDlg : public CDialog
{
// ����
public:
	CClientTestDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_CLIENTTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
