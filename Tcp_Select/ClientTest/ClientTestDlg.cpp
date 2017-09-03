
// ClientTestDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ClientTest.h"
#include "ClientTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CClientTestDlg �Ի���




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


// CClientTestDlg ��Ϣ�������

BOOL CClientTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// ��ʼ������
	SetDlgItemInt(IDC_EDIT_NUM,10000);
	SetDlgItemInt(IDC_EDIT_NET,100);
	m_pClient=NULL;
	CTcpClient::InitWinSocket();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CClientTestDlg::OnPaint()
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
HCURSOR CClientTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ����
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
	TRACE("-----���ӿ�ʼʱ��:%d-----\n",clock());
	for(int i=0;i<nNet;i++)
	{
		bool bRes=m_pClient[i].ConnectServer("127.0.0.1",6688);
		// Sleep(1);
	}
	TRACE("-----���ӽ���ʱ��:%d-----\n",clock());
}
// ����
void CClientTestDlg::OnBnClickedBtSend()
{
	char Pkg[81924];
	strcpy(Pkg+4,"123456");
	*((int*)Pkg)=81920;
	TRACE("-----���Ϳ�ʼʱ��:%d-----\n",clock());
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
			TRACE("-----���͵�%d�� ʱ��:%d-----\n",i,clock());
			Sleep(10);
		}
	}
	TRACE("-----���ͽ���ʱ��:%d-----\n",clock());
}
// ����
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
