
// ServerTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ServerTest.h"
#include "ServerTestDlg.h"

bool gRuning=true;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CServerTestDlg 对话框




CServerTestDlg::CServerTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CServerTestDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BT_START, &CServerTestDlg::OnBnClickedBtStart)
	ON_BN_CLICKED(IDC_BT_STOP, &CServerTestDlg::OnBnClickedBtStop)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BT_RESET, &CServerTestDlg::OnBnClickedBtReset)
END_MESSAGE_MAP()


// CServerTestDlg 消息处理程序

BOOL CServerTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// 初始化代码
	m_handle=NULL;
	CTcpServer::InitWinSocket();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CServerTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CServerTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CServerTestDlg::OnBnClickedBtStart()
{
	gRuning=true;
	if(m_Ser.CreateServer(6688))
	{
		GetDlgItem(IDC_BT_START)->EnableWindow(FALSE);
		DWORD dw;
		m_handle=CreateThread(NULL,0,LPTHREAD_START_ROUTINE(MonitorTh),this,0,&dw);
		if (NULL==m_handle)
		{
			MessageBox("启动监视线程失败!");
		}
	}
	else
	{
		MessageBox("开启服务失败!");
	}
}

// 监视函数
DWORD CServerTestDlg::MonitorTh(CServerTestDlg* lpVoid)
{
	SOCKET mSocket;
	char szIp[20];
	unsigned short nPort;
	while (gRuning)
	{
		mSocket=lpVoid->m_Ser.ListenConnect(szIp,&nPort);
		if (mSocket>0)
		{
			/*CString str;
			str.Format("连接的客户端IP:%s,端口:%d",szIp,nPort);
			WriteLog(str);*/
			// closesocket(mSocket);
			DWORD dw;
			HANDLE handle=CreateThread(NULL,0,LPTHREAD_START_ROUTINE(RecvTh),&mSocket,0,&dw);
			if (NULL!=handle)
			{
				CloseHandle(handle);
			}
			TRACE("-----客户端连接IP:%s,端口:%d,时间:%u-----\n",szIp,nPort,clock());
		}
	}
	return 0;
}

void CServerTestDlg::OnBnClickedBtStop()
{
	if (gRuning)
	{
		gRuning=false;
		m_Ser.StopServer();
		WaitForMultipleObjects(1,&m_handle, TRUE, INFINITE);
		GetDlgItem(IDC_BT_START)->EnableWindow(TRUE);
	}
}

void CServerTestDlg::OnClose()
{
	if (gRuning)
	{
		gRuning=false;
		m_Ser.StopServer();
		WaitForMultipleObjects(1,&m_handle, TRUE, INFINITE);
	}
	CTcpServer::CloseWinSocket();
	CDialog::OnClose();
}
int nIndex=1;
// 接收线程
DWORD CServerTestDlg::RecvTh(void* lpVoid)
{
	SOCKET nSocket=*(SOCKET*)lpVoid;
	char Pkg[81960]; //80K
	int nRecv=0,nLen=0;
	while(gRuning)
	{
		nRecv=CTcpServer::Recvn(nSocket,(char*)&nLen,4);
		switch (nRecv)
		{
		case 0:
			continue;
			break;
		case -1:
			closesocket(nSocket);
			nSocket=INVALID_SOCKET;
			TRACE("-----断开连接,时间:%u.-----\n",clock());
			return 0;
			break;
		default:
			//Sleep(1);
			//TRACE("-----收到数据长度:%d,时间:%d-----\n",nRecv,clock());
			break;
		}
		nRecv=CTcpServer::Recvn(nSocket,Pkg,nLen);
		switch (nRecv)
		{
		case 0:
			continue;
			break;
		case -1:
			closesocket(nSocket);
			nSocket=INVALID_SOCKET;
			TRACE("-----断开连接,时间:%u.-----\n",clock());
			return 0;
			break;
		default:
			//Sleep(1);
			TRACE("-----收到第%d笔:数据长度:%d,时间:%d-----\n",nIndex++,nRecv,clock());
			/*CTcpServer::Sendn(nSocket,(char*)&nLen,4);
			CTcpServer::Sendn(nSocket,Pkg,nLen);*/
			break;
		}
	}
	if (INVALID_SOCKET!=nSocket)
	{
		closesocket(nSocket);
	}
	return 0;
}

void CServerTestDlg::OnBnClickedBtReset()
{
	nIndex=1;
}
