
#include "pch.h"
#include "utils.h"
#include "resource.h"
wchar_t LOGFILENAME[1024]{ L"log.txt" };
const wchar_t* DATEFORMAT = L"%Y%m%d%H%M%S";


CTime GetFileDateTime(LPCWSTR FileName)
{
	FILETIME ftCreate, ftAccess, ftWrite;
	HANDLE hFile;
	CTime result = NULL;
	CTime FileTime;

	hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return result;
	}

	// Retrieve the file times for the file.
	if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
		return result;
	FileTime = ftWrite;

	CloseHandle(hFile);
	result = (CTime)FileTime;
	return result;
}

CTime Time2UTC(CTime original)
{
	CString Formatted = original.FormatGmt(DATEFORMAT);
	int Year, Month, Day, Hour, Minute;

	if (Formatted != L"" && Formatted.GetLength() >= 12)
	{
		Year = _wtol(Formatted.Left(4));
		Month = _wtol(Formatted.Mid(4, 2));
		Day = _wtol(Formatted.Mid(6, 2));
		Hour = _wtol(Formatted.Mid(8, 2));
		Minute = _wtol(Formatted.Mid(10, 2));
		CTime result(Year, Month, Day, Hour, Minute, 0);
		return result;
	}
	else
	{
		return (CTime)NULL;
	}
}

#include "res\version.h"
#pragma comment(lib, "Version.lib")

BOOL SG_GetVersion(LPWSTR ExeFile, SG_Version* ver)
{
	BOOL result = FALSE;
	DWORD dwDummy;
	DWORD dwFVISize = GetFileVersionInfoSize(ExeFile, &dwDummy);
	LPBYTE lpVersionInfo = new BYTE[dwFVISize];
	GetFileVersionInfo(ExeFile, 0, dwFVISize, lpVersionInfo);
	UINT uLen;
	VS_FIXEDFILEINFO* lpFfi;
	VerQueryValue(lpVersionInfo, _T("\\"), (LPVOID*)&lpFfi, &uLen);
	if (lpFfi && uLen)
	{
		DWORD dwFileVersionMS = lpFfi->dwFileVersionMS;
		DWORD dwFileVersionLS = lpFfi->dwFileVersionLS;
		delete[] lpVersionInfo;
		ver->Major = HIWORD(dwFileVersionMS);
		ver->Minor = LOWORD(dwFileVersionMS);
		ver->Revision = HIWORD(dwFileVersionLS);
		ver->SubRevision = LOWORD(dwFileVersionLS);
		result = TRUE;
	}
	return result;
}
CTime GetSelfDateTimeGMT(void)
{
	WCHAR szExeFileName[MAX_PATH];
	GetModuleFileName(NULL, szExeFileName, MAX_PATH);
	return Time2UTC(GetFileDateTime(szExeFileName));
}
#define FRIENDLY_DATEFORMAT L"%m-%d-%Y, %H:%M:%S UTC"

CString GetCreationDateTime()
{
	CString message{ L"" };
	message = GetSelfDateTimeGMT().Format(FRIENDLY_DATEFORMAT);
	return message;
}


// Helper function to convert std::wstring to std::string using Windows API
std::string wstring_to_string(const std::wstring& wstr)
{
	if (wstr.empty())
	{
		return "";
	}

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string str(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL);

	return str;
}

// Helper function to convert std::string to std::wstring using Windows API
std::wstring string_to_wstring(const std::string& str)
{
	if (str.empty())
	{
		return L"";
	}

	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstr(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);

	return wstr;
}

#include <atlstr.h>
#include <strsafe.h>
#include <Shellapi.h>

// Function to display a balloon notification
void NotifyVersionInfo(CString title, CString text)
{
	static bool ongoing{false};
	static CString last_text;
	if(ongoing) return;
	last_text = text;
	ongoing = true;
	// Create a NOTIFYICONDATA structure
	NOTIFYICONDATA nid = {};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd(); // Get the main window handle
	nid.uID = 1;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	nid.uCallbackMessage = WM_USER + 1;
	nid.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), _T("Notification"));

	// Set the balloon information
	StringCchCopy(nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle), title);
	StringCchCopy(nid.szInfo, ARRAYSIZE(nid.szInfo), text);
	nid.dwInfoFlags = NIIF_INFO; // NIIF_INFO for an information icon, you can also use NIIF_WARNING, NIIF_ERROR, etc.
	nid.uTimeout = 5000; // Display for 5 seconds

	// Add the icon to the system tray
	Shell_NotifyIcon(NIM_ADD, &nid);

	// Display the balloon notification
	//Shell_NotifyIcon(NIM_MODIFY, &nid);

	// Sleep to allow the balloon to display for the specified time (optional, but should be managed properly in actual use)
	Sleep(nid.uTimeout);

	// Remove the icon from the system tray
	Shell_NotifyIcon(NIM_DELETE, &nid);
	ongoing = false;
}


void ShowError(HRESULT errorCode)
{
	LPVOID lpMsgBuf = nullptr;
	DWORD dwChars = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);
	if (lpMsgBuf)
	{
		WriteLogFile(L"Error: %s", (LPTSTR)lpMsgBuf);
	}
	else
	{
		WriteLogFile(L"Error: %d (unknown)", errorCode);
	}
}

CAtlString CurrentDate()
{
	CTime currentDateTime = CTime::GetCurrentTime();
	// Do we need to convert the local time into GMT time?
	//currentDateTime.GetGmtTm(&gmt);
	return currentDateTime.FormatGmt(DATEFORMAT);
}
std::wstring CurrentTimeForUpdate()
{
	time_t currTime;
	time(&currTime);

	//write in required format
	wchar_t buffer[16];
	std::wcsftime(buffer, 16, L"%Y%m%d%H%M%S", std::localtime(&currTime));
	std::wstring yyyymmddhhmmss(buffer);
	return yyyymmddhhmmss;
}



std::wstring removeSubstring(const std::wstring& original, const std::wstring& substringToRemove)
{
	std::wstring result = original;

	// Find the position of the substring
	size_t substringPos = result.find(substringToRemove);

	// Check if the substring is found
	if (substringPos != std::wstring::npos)
	{
		// Remove the substring
		result.erase(substringPos, substringToRemove.length());
	}

	return result;
}

void OpenLogFile()
{
	ShellExecute(GetForegroundWindow(), L"OPEN", LOGFILENAME, NULL, NULL, TRUE);
}
// Function:	WriteLogFile
// Purpose:		Print a message to the log file and (in DEBUG) to the Console

void DeleteLogFile()
{
	//DeleteFile(LOGFILENAME);
}

CDialog* m_globalDlg;

void ShowStatus(LPCWSTR lpText)
{
	m_globalDlg->GetDlgItem(IDC_STATUS)->SetWindowTextW(lpText);
}

std::wstring GetExecutablePath()
{
	wchar_t path[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, path, MAX_PATH);
	std::wstring executablePath(path);

	// Find the last backslash
	size_t lastBackslashPos = executablePath.find_last_of(L"\\/");
	if (lastBackslashPos != std::wstring::npos)
	{
		// Extract the directory path including the trailing backslash
		return executablePath.substr(0, lastBackslashPos + 1);
	}

	// If not found, return the path as is
	return executablePath;
}

int WriteLogFile(LPCWSTR lpText, ...)
{
	std::wofstream ofs;
	CTime Today = CTime::GetCurrentTime();

	CAtlStringW sMsg;
	va_list ptr;
	va_start(ptr, lpText);
	sMsg.FormatV(lpText, ptr);
	va_end(ptr);
	wprintf(L"%s\n", sMsg.GetString());
	ShowStatus(sMsg.GetString());
	try
	{
		ofs.open(GetExecutablePath()+LOGFILENAME, std::ios_base::app);
		if (ofs.is_open())
		{
			ofs.imbue(std::locale("en_US.utf8")); // Set UTF-8 locale
			ofs << Today.FormatGmt(L"%d.%m.%Y %H:%M").GetString() << L": " << sMsg.GetString() << L"\n";
			ofs.close(); // Close the file after writing
		}
	}
	catch (const std::exception& e)
	{
		// Log the exception
		CStringA errorMsg;
		errorMsg.Format("Exception occurred while writing to log file: %s\n", e.what());
		OutputDebugStringA(errorMsg);
		return FALSE;
	}

	return TRUE;
}
