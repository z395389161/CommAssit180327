
// CommAssit.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CCommAssitApp:
// �йش����ʵ�֣������ CommAssit.cpp
//

class CCommAssitApp : public CWinAppEx
{
public:
	CCommAssitApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CCommAssitApp theApp;