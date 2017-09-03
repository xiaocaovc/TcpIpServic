
// ServerTestDlg.h : ͷ�ļ�
//

#pragma once
#include "TcpServer.h"
#include <fstream>
#include <locale>


// CServerTestDlg �Ի���
class CServerTestDlg : public CDialog
{
// ����
public:
	CServerTestDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SERVERTEST_DIALOG };

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
private:
	static void WriteLog(CString LogText)
	{
		//д�ļ�����
		static CString FilePath=_T("");
		if (FilePath==_T(""))
		{
			char szFilePath[_MAX_PATH];
			::GetModuleFileName(NULL, szFilePath, _MAX_PATH);
			FilePath=szFilePath;
			FilePath=FilePath.Left(FilePath.ReverseFind('\\'));
			FilePath+=_T("\\Log");
		}
		if (GetFileAttributes(FilePath) != FILE_ATTRIBUTE_DIRECTORY)//Ŀ¼�����ڴ���
		{
			CreateDirectory(FilePath, NULL);
		}
		std::ofstream logFile;
		CString Log;
		CTime currentTime = CTime::GetCurrentTime();//COleDateTime
		Log.Format("%s\\%s.log",FilePath,currentTime.Format(_T("%Y%m%d%H")));//��ÿСʱ����Log
		std::locale::global(std::locale(""));//��ȫ��������Ϊ����ϵͳĬ������--���������ļ���
		logFile.open(Log, std::ios::app);
		std::locale::global(std::locale("C"));//��ԭȫ�������趨
		Log.Format(_T("%s [Log]:%s"),currentTime.Format(_T("%Y-%m-%d %H:%M:%S")),LogText);
		logFile << Log;
		logFile <<"\n";
		logFile.close();
	}
public:
	afx_msg void OnBnClickedBtStart();
	// ���Ӻ���
	static DWORD MonitorTh(CServerTestDlg* lpVoid);
	CTcpServer m_Ser;
	HANDLE m_handle;
	afx_msg void OnBnClickedBtStop();
	afx_msg void OnClose();
	// �����߳�
	static DWORD RecvTh(void* lpVoid);
	afx_msg void OnBnClickedBtReset();
};
