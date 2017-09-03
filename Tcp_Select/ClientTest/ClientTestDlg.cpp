
// ClientTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ClientTest.h"
#include "ClientTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CClientTestDlg 对话框




CClientTestDlg::CClientTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CClientTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CClientTestDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BT_CONN, &CClientTestDlg::OnBnClickedBtConn)
	ON_BN_CLICKED(IDC_BT_SEND, &CClientTestDlg::OnBnClickedBtSend)
	ON_BN_CLICKED(IDC_BT_RECV, &CClientTestDlg::OnBnClickedBtRecv)
	ON_BN_CLICKED(IDC_BT_STOP, &CClientTestDlg::OnBnClickedBtStop)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CClientTestDlg 消息处理程序

BOOL CClientTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// 初始化代码
	SetDlgItemInt(IDC_EDIT_NUM,10000);
	SetDlgItemInt(IDC_EDIT_NET,100);
	m_pClient=NULL;
	CTcpClient::InitWinSocket();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CClientTestDlg::OnPaint()
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
HCURSOR CClientTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 连接
void CClientTestDlg::OnBnClickedBtConn()
{
	if (m_pClient)
	{
		return;
	}
	GetDlgItem(IDC_BT_CONN)->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_SEND)->EnableWindow(TRUE);
	GetDlgItem(IDC_BT_RECV)->EnableWindow(TRUE);
	GetDlgItem(IDC_BT_STOP)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_NET)->EnableWindow(FALSE);
	int nNet=GetDlgItemInt(IDC_EDIT_NET);
	m_pClient=new CTcpClient[nNet];
	TRACE("-----连接开始时间:%d-----\n",clock());
	for(int i=0;i<nNet;i++)
	{
		bool bRes=m_pClient[i].ConnectServer("127.0.0.1",6688);
		// Sleep(1);
	}
	TRACE("-----连接结束时间:%d-----\n",clock());
}
// 发送
void CClientTestDlg::OnBnClickedBtSend()
{
	char Pkg[81924];
	strcpy(Pkg+4,"123456");
	*((int*)Pkg)=81920;
	TRACE("-----发送开始时间:%d-----\n",clock());
	int Num=GetDlgItemInt(IDC_EDIT_NUM);
	int nNet=GetDlgItemInt(IDC_EDIT_NET);
	for (int i=0;i<Num;)
	{
		for (int j=0;j<nNet;j++)
		{
			if(0==m_pClient[j].Sendn(Pkg,81924))
			{
				continue;
			}
			i++;
			TRACE("-----发送第%d笔 时间:%d-----\n",i,clock());
			Sleep(10);
		}
	}
	TRACE("-----发送结束时间:%d-----\n",clock());
}
// 接收
void CClientTestDlg::OnBnClickedBtRecv()
{
	char Pkg[81924];
	int nLen=0;
	int nRes=m_pClient[0].Recvn((char*)&nLen,4);
	if (nRes==-1)
	{
		return;
	}
	nRes=m_pClient[0].Recvn(Pkg,nLen);
}

void CClientTestDlg::OnBnClickedBtStop()
{
	GetDlgItem(IDC_EDIT_NET)->EnableWindow(TRUE);
	if (!m_pClient)
	{
		return;
	}
	int nNet=GetDlgItemInt(IDC_EDIT_NET);
	for (int i=0;i< nNet;i++)
	{
		m_pClient[i].StopClient();
	}
	delete[] m_pClient;
	m_pClient=NULL;
	GetDlgItem(IDC_BT_CONN)->EnableWindow(TRUE);
	GetDlgItem(IDC_BT_SEND)->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_RECV)->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_STOP)->EnableWindow(FALSE);
}

void CClientTestDlg::OnClose()
{
	if (m_pClient)
	{
		int nNet=GetDlgItemInt(IDC_EDIT_NET);
		for (int i=0;i<nNet;i++)
		{
			m_pClient[i].StopClient();
		}
		delete[] m_pClient;
		m_pClient=NULL;
	}
	CTcpClient::CloseWinSocket();
	CDialog::OnClose();
}
