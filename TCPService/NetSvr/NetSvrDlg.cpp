
// NetSvrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "NetSvr.h"
#include "NetSvrDlg.h"
#include "afxdialogex.h"
#pragma comment(lib,"ws2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CNetSvrDlg 对话框



CNetSvrDlg::CNetSvrDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_NETSVR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNetSvrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CNetSvrDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BT_START, &CNetSvrDlg::OnBnClickedBtStart)
END_MESSAGE_MAP()


// CNetSvrDlg 消息处理程序

BOOL CNetSvrDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CNetSvrDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CNetSvrDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CNetSvrDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HANDLE hCompletionPort;
typedef struct _PER_HANDLE_DATA
{
	SOCKET sock;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

LPPER_HANDLE_DATA perHandleData;
LPFN_ACCEPTEX m_lpfnAcceptEx;
LPFN_GETACCEPTEXSOCKADDRS m_lpfnGetAcceptExSockAddrs;

OVERLAPPED   m_Overlapped;

char m_Buff[(sizeof(SOCKADDR_IN) + 16) * 2];
// 开始
void CNetSvrDlg::OnBnClickedBtStart()
{
	net_CreateServer(12345, NULL);
	/*m_Svr.Init(12345);*/
	return;
	WSADATA wsaData;
	// 错误(一般都不可能出现)
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		TRACE("---初始化WinSock 2.2失败!---\n");
	}
	// 建立监视Socket
	m_Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_Socket)
	{
		TRACE(_T("初始化监视Socket失败，错误代码: %d."), WSAGetLastError());
		return;
	}
	int nReuseAddr = 1;
	/*if (setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&nReuseAddr, sizeof(int)) != 0)
	{
	return;
	}*/
	// 服务器地址信息，用于绑定Socket
	struct sockaddr_in ServerAddress;
	// 填充地址信息
	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	// 这里可以绑定任何可用的IP地址，或者绑定一个指定的IP地址
	/*if (szSerIp)
	{
	ServerAddress.sin_addr.s_addr = inet_addr(szSerIp);
	}
	else
	{*/
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	//}
	ServerAddress.sin_port = htons(12345);
	// 绑定地址和端口
	if (SOCKET_ERROR == bind(m_Socket, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)))
	{
		TRACE("bind()函数执行错误！");
		return;
	}
	// 开始进行监听
	if (SOCKET_ERROR == listen(m_Socket, SOMAXCONN))
	{
		TRACE("监听失败！错误代码: %d.", WSAGetLastError());
		return;
	}

	hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hCompletionPort == INVALID_HANDLE_VALUE)
	{
		return;
	}
	if (NULL == CreateIoCompletionPort((HANDLE)m_Socket, hCompletionPort,
		(DWORD)(this), 0))
	{
		TRACE(_T("绑定 Listen Socket至完成端口失败！错误代码: %d！"), WSAGetLastError());
		return;
	}
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	// 获取AcceptEx函数指针
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(m_Socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx,
		sizeof(GuidAcceptEx),
		&m_lpfnAcceptEx,
		sizeof(LPFN_ACCEPTEX),
		&dwBytes,
		NULL,
		NULL))
	{
		TRACE("WSAIoctl 未能获取AcceptEx函数指针。错误代码: %d!!", WSAGetLastError());
		//Log_WriteLog(1, "CTcpServer::Init WSAIoctl 未能获取AcceptEx函数指针!错误代码: %d.", WSAGetLastError());
		return;
	}
	//#ifdef _DEBUG
	// 获取GetAcceptExSockAddrs函数指针，也是同理
	if (SOCKET_ERROR == WSAIoctl(m_Socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs),
		&m_lpfnGetAcceptExSockAddrs,
		sizeof(m_lpfnGetAcceptExSockAddrs),
		&dwBytes,
		NULL,
		NULL))
	{
		TRACE("WSAIoctl 未能获取GuidGetAcceptExSockAddrs函数指针。错误代码: %d!!", WSAGetLastError());
		//Log_WriteLog(1, "CTcpServer::Init WSAIoctl 未能获取uidGetAcceptExSockAddrs函数指针!错误代码: %d.", WSAGetLastError());
		return;
	}
	// Accept
	SOCKET sockAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	ZeroMemory(&m_Overlapped, sizeof(OVERLAPPED));
	if (!m_lpfnAcceptEx(m_Socket, sockAccept,
		&m_Buff, 0/*p_wbuf->len - ((sizeof(SOCKADDR_IN)+16)*2*/,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &m_Overlapped))
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			TRACE("-----投递 AcceptEx 请求失败，错误代码: %d-----", WSAGetLastError());
			return;
		}
	}
}
