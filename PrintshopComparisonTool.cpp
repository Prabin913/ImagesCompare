
// PrintshopComparisonTool.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "PrintshopComparisonTool.h"
#include "PrintshopComparisonToolDlg.h"
#include "utils.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CPrintshopComparisonToolApp

BEGIN_MESSAGE_MAP(CPrintshopComparisonToolApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()



// CPrintshopComparisonToolApp construction

CPrintshopComparisonToolApp::CPrintshopComparisonToolApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CPrintshopComparisonToolApp object

CPrintshopComparisonToolApp theApp;



BOOL CPrintshopComparisonToolApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	


#ifdef _DEBUG
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
#endif

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	PrintshopComparisonToolDlg dlg;
	m_pMainWnd = &dlg;


	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Access the command line arguments
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
	{
		// No command line arguments provided
	}
	else if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen)
	{
		// Command line parameters provided
		CString strFileName = cmdInfo.m_strFileName;
		// You can now use strFileName as the command line argument.
		
	}


	// Check for batch file command line argument
	if (cmdInfo.m_strFileName.GetLength() > 0 && cmdInfo.m_strFileName.Left(2) == L"-b")
	{
		// Extract batch file path
		CString strOption = cmdInfo.m_strFileName.Mid(2).Trim();
		if (!strOption.IsEmpty())
		{
			// Verify if the file exists
			if (PathFileExists(strOption))
			{
				// Set the batch file path in the dialog
				dlg.m_batchFile = strOption;
			}
			else
			{
				// Handle error: Batch file does not exist
				AfxMessageBox(L"Error: Batch file specified does not exist.");
				return FALSE; // Exit application or handle as necessary
			}
		}
	}

	CWnd* pParentWnd = 0;
	pParentWnd = CWnd::FromHandle(m_pMainWnd->GetSafeHwnd());
	m_globalDlg = (CDialog *)&dlg;
	// set batch file
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
	}
	delete pParentWnd;

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

