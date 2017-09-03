
// NetSvrDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "NetSvr.h"
#include "NetSvrDlg.h"
#include "afxdialogex.h"
#pragma comment(lib,"ws2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CNetSvrDlg �Ի���



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


// CNetSvrDlg ��Ϣ�������

BOOL CNetSvrDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CNetSvrDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
// ��ʼ
void CNetSvrDlg::OnBnClickedBtStart()
{
	net_CreateServer(12345, NULL);
	/*m_Svr.Init(12345);*/
	return;
	WSADATA wsaData;
	// ����(һ�㶼�����ܳ���)
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		TRACE("---��ʼ��WinSock 2.2ʧ��!---\n");
	}
	// ��������Socket
	m_Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_Socket)
	{
		TRACE(_T("��ʼ������Socketʧ�ܣ��������: %d."), WSAGetLastError());
		return;
	}
	int nReuseAddr = 1;
	/*if (setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&nReuseAddr, sizeof(int)) != 0)
	{
	return;
	}*/
	// ��������ַ��Ϣ�����ڰ�Socket
	struct sockaddr_in ServerAddress;
	// ����ַ��Ϣ
	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	// ������԰��κο��õ�IP��ַ�����߰�һ��ָ����IP��ַ
	/*if (szSerIp)
	{
	ServerAddress.sin_addr.s_addr = inet_addr(szSerIp);
	}
	else
	{*/
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	//}
	ServerAddress.sin_port = htons(12345);
	// �󶨵�ַ�Ͷ˿�
	if (SOCKET_ERROR == bind(m_Socket, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)))
	{
		TRACE("bind()����ִ�д���");
		return;
	}
	// ��ʼ���м���
	if (SOCKET_ERROR == listen(m_Socket, SOMAXCONN))
	{
		TRACE("����ʧ�ܣ��������: %d.", WSAGetLastError());
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
		TRACE(_T("�� Listen Socket����ɶ˿�ʧ�ܣ��������: %d��"), WSAGetLastError());
		return;
	}
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	// ��ȡAcceptEx����ָ��
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
		TRACE("WSAIoctl δ�ܻ�ȡAcceptEx����ָ�롣�������: %d!!", WSAGetLastError());
		//Log_WriteLog(1, "CTcpServer::Init WSAIoctl δ�ܻ�ȡAcceptEx����ָ��!�������: %d.", WSAGetLastError());
		return;
	}
	//#ifdef _DEBUG
	// ��ȡGetAcceptExSockAddrs����ָ�룬Ҳ��ͬ��
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
		TRACE("WSAIoctl δ�ܻ�ȡGuidGetAcceptExSockAddrs����ָ�롣�������: %d!!", WSAGetLastError());
		//Log_WriteLog(1, "CTcpServer::Init WSAIoctl δ�ܻ�ȡuidGetAcceptExSockAddrs����ָ��!�������: %d.", WSAGetLastError());
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
			TRACE("-----Ͷ�� AcceptEx ����ʧ�ܣ��������: %d-----", WSAGetLastError());
			return;
		}
	}
}
