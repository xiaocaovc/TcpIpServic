
// ServerTestDlg.h : 头文件
//

#pragma once
#include "TcpServer.h"
#include <fstream>
#include <locale>


// CServerTestDlg 对话框
class CServerTestDlg : public CDialog
{
// 构造
public:
	CServerTestDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SERVERTEST_DIALOG };

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
private:
	static void WriteLog(CString LogText)
	{
		//写文件操作
		static CString FilePath=_T("");
		if (FilePath==_T(""))
		{
			char szFilePath[_MAX_PATH];
			::GetModuleFileName(NULL, szFilePath, _MAX_PATH);
			FilePath=szFilePath;
			FilePath=FilePath.Left(FilePath.ReverseFind('\\'));
			FilePath+=_T("\\Log");
		}
		if (GetFileAttributes(FilePath) != FILE_ATTRIBUTE_DIRECTORY)//目录不存在创建
		{
			CreateDirectory(FilePath, NULL);
		}
		std::ofstream logFile;
		CString Log;
		CTime currentTime = CTime::GetCurrentTime();//COleDateTime
		Log.Format("%s\\%s.log",FilePath,currentTime.Format(_T("%Y%m%d%H")));//按每小时生成Log
		std::locale::global(std::locale(""));//将全局区域设为操作系统默认区域--用于中文文件夹
		logFile.open(Log, std::ios::app);
		std::locale::global(std::locale("C"));//还原全局区域设定
		Log.Format(_T("%s [Log]:%s"),currentTime.Format(_T("%Y-%m-%d %H:%M:%S")),LogText);
		logFile << Log;
		logFile <<"\n";
		logFile.close();
	}
public:
	afx_msg void OnBnClickedBtStart();
	// 监视函数
	static DWORD MonitorTh(CServerTestDlg* lpVoid);
	CTcpServer m_Ser;
	HANDLE m_handle;
	afx_msg void OnBnClickedBtStop();
	afx_msg void OnClose();
	// 接收线程
	static DWORD RecvTh(void* lpVoid);
	afx_msg void OnBnClickedBtReset();
};
