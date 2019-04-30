
// ResourceManager.h : main header file for the ResourceManager application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CResourceManagerApp:
// See ResourceManager.cpp for the implementation of this class
//

class CResourceManagerApp : public CWinApp
{
public:
	CResourceManagerApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CResourceManagerApp theApp;
