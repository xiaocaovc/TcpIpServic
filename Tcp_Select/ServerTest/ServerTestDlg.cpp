
// ServerTestDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ServerTest.h"
#include "ServerTestDlg.h"

bool gRuning=true;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CServerTestDlg �Ի���




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


// CServerTestDlg ��Ϣ�������

BOOL CServerTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// ��ʼ������
	m_handle=NULL;
	CTcpServer::InitWinSocket();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CServerTestDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
			MessageBox("���������߳�ʧ��!");
		}
	}
	else
	{
		MessageBox("��������ʧ��!");
	}
}

// ���Ӻ���
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
			str.Format("���ӵĿͻ���IP:%s,�˿�:%d",szIp,nPort);
			WriteLog(str);*/
			// closesocket(mSocket);
			DWORD dw;
			HANDLE handle=CreateThread(NULL,0,LPTHREAD_START_ROUTINE(RecvTh),&mSocket,0,&dw);
			if (NULL!=handle)
			{
				CloseHandle(handle);
			}
			TRACE("-----�ͻ�������IP:%s,�˿�:%d,ʱ��:%u-----\n",szIp,nPort,clock());
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
// �����߳�
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
			TRACE("-----�Ͽ�����,ʱ��:%u.-----\n",clock());
			return 0;
			break;
		default:
			//Sleep(1);
			//TRACE("-----�յ����ݳ���:%d,ʱ��:%d-----\n",nRecv,clock());
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
			TRACE("-----�Ͽ�����,ʱ��:%u.-----\n",clock());
			return 0;
			break;
		default:
			//Sleep(1);
			TRACE("-----�յ���%d��:���ݳ���:%d,ʱ��:%d-----\n",nIndex++,nRecv,clock());
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
