// TcpClient.h : TcpClient DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTcpClientApp
// �йش���ʵ�ֵ���Ϣ������� TcpClient.cpp
//

class CTcpClientApp : public CWinApp
{
public:
	CTcpClientApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
