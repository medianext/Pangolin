
// Pangolin.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CPangolinApp: 
// �йش����ʵ�֣������ Pangolin.cpp
//

class CPangolinApp : public CWinApp
{
public:
	CPangolinApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CPangolinApp theApp;