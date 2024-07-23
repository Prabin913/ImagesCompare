#pragma once
UINT CALLBACK OfnHookProc(HWND hDlg, UINT uMsg, UINT wParam, LONG lParam);

class SGAvbFileBrowse
{
private:
	OPENFILENAME ofn;

public:
	CString GetAVBFile();

};

