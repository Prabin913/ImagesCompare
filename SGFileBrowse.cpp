#include "pch.h"
#include "resource.h"
#include "PrintshopComparisonToolDlg.h"
#include "utils.h"
#include "BatchViewer.h"
#include "SGFileBrowse.h"

CComboBox * pCombo = NULL;
TCHAR szPathName[_MAX_PATH] = { 0 };

wchar_t szPathBuffer[2048] = { 0 };


void AddFileOrFolderToBatch(LPWSTR FileName, BOOL AddFolder)
 {
	BOOL FolderMode = FALSE;
	if (PathFileExists(FileName) == NULL)
	{
		//SG_MessageBox(L"Invalid path " + (CString)FileName, L"Batch Manager", MB_OK);
		return;
	}
	// Check if the path is an PDF / PNG file or a folder
	CString ext = ((CString)(FileName)).Right(3).MakeUpper();
	FolderMode = (ext == L"PDF" || ext == L"PNG") ? FALSE : TRUE;

	if (FolderMode == TRUE && AddFolder == FALSE)	// Selecting a folder and asking to add a file
	{
		return;
	}
	if (FolderMode == FALSE && AddFolder == TRUE)	// Selecting a file and asking to add a folder
													// in that case we add the file's folder

	{
		CString myPath = SG_GetBaseFilePath(FileName);
		wcscpy(FileName, myPath.GetBuffer());
	}

	ext = ((CString)(FileName)).Right(3).MakeUpper();
	FolderMode = (ext == L"PDF" || ext == L"PNG") ? FALSE : TRUE;
	if ((FileName[2] == L'\\') && (wcslen(FileName) == 3))
		FileName[2] = L'\0';

	//LoadSettings(MySettings);
	//CString strSaved = MySettings.Last_Trsc_Lang.c_str();

	//BatchManager myBatchManager;
	//SG_Batch item;
	//wcscpy(item.file_path, FileName);
	//wcscpy(item.language, strSaved.GetBuffer());
	//item.clip_index = -1; //  0; 0 means first clip. -1 means no specific clip / all clips
	//item.entity_type = (AddFolder)? (EntityType::Folder):(EntityType::Avbfile);
	//item.status = 0;

	//if (myBatchManager.ItemExists(item))
	//{
	//	SG_MessageBox(((FolderMode)?L"Path ":L"File ")+(CString)FileName + L" already exists in our batch", L"Batch Manager", MB_OK);
	//}
	//else
	//{
	//	myBatchManager.AddItem(item);
	//	SG_MessageBox(((FolderMode) ? L"Path " : L"File ") + (CString)FileName + (CString)L" was added " +  L" To batch", L"Batch Manager", MB_OK);
	//
	//	// update batch viewer
	//	CMatchPMR_AVBMainDlg* pMainDlg = (CMatchPMR_AVBMainDlg*)AfxGetApp()->GetMainWnd();
	//	if (pMainDlg)
	//		pMainDlg->m_pBatchV->PostMessage(WM_USER_BATCH_REFRESH, 0, 0);
	//}
}


UINT CALLBACK OfnHookProc(HWND hDlg, UINT uMsg, UINT wParam, LONG lParam)
{
	OFNOTIFY* pofNotify{ nullptr };
	HWND hTrueDlg;
	switch (uMsg)
	{
		case WM_INITDIALOG:
			pCombo = (CComboBox *)GetDlgItem(hDlg, IDC_CMB_BATCHOPTION);
			ShowWindow((HWND)pCombo, TRUE);
			SendMessage((HWND)pCombo, CB_ADDSTRING, 0, (LPARAM)L"Select a Batch option...");

			SendMessage((HWND)pCombo, CB_ADDSTRING, 0, (LPARAM)L"Add file to Batch");
			SendMessage((HWND)pCombo, CB_ADDSTRING, 0, (LPARAM)L"Add folder to Batch");
			SendMessage((HWND)pCombo, CB_SETCURSEL, 0, (LPARAM)0);
			break;
			// CB_SETCURSEL
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDC_CMB_BATCHOPTION)
			{
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL,
						(WPARAM)0, (LPARAM)0);
					TCHAR  ListItem[256];
					(TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT,
						(WPARAM)ItemIndex, (LPARAM)ListItem);
					(TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT,
						(WPARAM)ItemIndex, (LPARAM)ListItem);

					if (ItemIndex == 1)	// Add file to batch
					{
						if (wcscmp(szPathName, L"") == NULL)
						{
							return (TRUE);
						}
						AddFileOrFolderToBatch(szPathName,FALSE);
						SendMessage((HWND)pCombo, CB_SETCURSEL, 0, (LPARAM)0);

					}
					if (ItemIndex == 2)	// Add folder to batch
					{
						if (wcscmp(szPathName, L"") == NULL)
						{
							return (TRUE);
						}

						AddFileOrFolderToBatch(szPathName,TRUE);
						SendMessage((HWND)pCombo, CB_SETCURSEL, 0, (LPARAM)0);

					}
				}
				return (TRUE);
			}
			break;
		}
		/*
		case WM_NOTIFY:
			pofNotify = (OFNOTIFY*)lParam;
			switch (pofNotify->hdr.code)
			{
				case CBN_SELCHANGE:
					return (TRUE);
			  
				case CDN_INITDONE:
					return (TRUE);
				 
				case CDN_SELCHANGE:
					hTrueDlg = GetParent(hDlg);
					SendMessage(hTrueDlg, CDM_GETFILEPATH, _MAX_PATH, (LONG)szPathName);
					SetDlgItemText(hDlg, IDC_STA_CURPATH, szPathName);
					wcscpy(szPathBuffer, szPathName);
					return (TRUE);
				  
				case CDN_FOLDERCHANGE:
					hTrueDlg = GetParent(hDlg);
					SendMessage(hTrueDlg, CDM_GETFOLDERPATH, _MAX_PATH, (LONG)szPathName);
					SetDlgItemText(hDlg, IDC_STA_CURPATH, szPathName);
					wcscpy(szPathBuffer, szPathName);

					return (TRUE);
				 
				case CDN_FILEOK:
					return TRUE;
				default:
					return FALSE;
			}
			*/
		default:
			return (FALSE);
	}
}
CString SGAvbFileBrowse::GetInputFile()
{
	CMenu m_ContextMenu;
	//LoadSettings(MySettings);
	//wcscpy(szPathBuffer, MySettings.Last_Browse_Path.c_str());
	//if (wcscmp(szPathBuffer, L"") == NULL)
	//{
		GetCurrentDirectory(256, (LPWSTR)&szPathBuffer[0]);
	//}
	// load menu
	m_ContextMenu.LoadMenuW(IDR_MENU_AVBFILES);

	OPENFILENAME ofn;       // common dialog box structure
	wchar_t szFile[260];     // buffer for file name
	HWND hwnd;              // owner window
	HANDLE hf;              // file handle

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	ofn.lpstrFile = szFile;
	ofn.lpTemplateName = MAKEINTRESOURCE(IDD_BATCHOPTIONS);
	ofn.hInstance = AfxGetInstanceHandle();
	ofn.lpfnHook = (LPOFNHOOKPROC) OfnHookProc;
	ofn.nMaxFile = 260;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"All\0*.*\0PDF\0*.pdf\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;

	ofn.lpstrInitialDir = szPathBuffer;
	ofn.Flags = OFN_EXPLORER |  /* chicago-style dialog box */
		OFN_FILEMUSTEXIST |       /* can't choose non-existing file */
		OFN_HIDEREADONLY |        /* hide "read-only" checkbox */
		OFN_ENABLEHOOK |          /* use hook function */
		OFN_ENABLETEMPLATE;       /* custom template resource */



								  
								  /*
	ofn.Flags = OFN_ENABLETEMPLATE | OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ENABLEHOOK;
	*/	
	
	if (GetOpenFileName(&ofn))
	{

		DestroyMenu(m_ContextMenu);
		//MySettings.Last_Browse_Path = SG_GetBaseFilePath(szPathBuffer);
		//SaveSettings(MySettings);
		return ofn.lpstrFile;
	}
	DestroyMenu(m_ContextMenu);
	//MySettings.Last_Browse_Path = SG_GetBaseFilePath(szPathBuffer);
	//SaveSettings(MySettings);
	return L"";
}
