
// PrintshopComparisonTool.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif
#ifndef _DEBUG
#endif

#include "resource.h"		// main symbols
class CSecuredGlobe_PRS_Class
{
public:
};

// CPrintshopComparisonToolApp:
// See PrintshopComparisonTool.cpp for the implementation of this class
//

class CPrintshopComparisonToolApp : public CWinApp
{
public:
	CPrintshopComparisonToolApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CPrintshopComparisonToolApp theApp;
int WriteLogFile(LPCWSTR lpText, ...);
