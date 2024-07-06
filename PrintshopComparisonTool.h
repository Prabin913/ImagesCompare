
// PrintshopComparisonTool.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif
#ifndef _DEBUG
#endif

#include "resource.h"		// main symbols

#define BG_COLOR		RGB(215, 217, 239)
#define	TX_COLOR		RGB(25, 19, 31)
#define	TX_COLOR2		RGB(64, 6, 49)
#define	TX_COLOR3		RGB(0, 0, 0)
#define	TX_HOVER_COLOR	RGB(180, 151, 209)

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
