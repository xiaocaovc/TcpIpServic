// TCPService.h : TCPService DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTCPServiceApp
// �йش���ʵ�ֵ���Ϣ������� TCPService.cpp
//

class CTCPServiceApp : public CWinApp
{
public:
	CTCPServiceApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
