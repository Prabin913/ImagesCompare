
#include "pch.h"
#include "utils.h"
wchar_t LOGFILENAME[1024]{ L"log.txt" };
const wchar_t* DATEFORMAT = L"%Y%m%d%H%M%S";


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
#include <fstream>

#include <fstream>
#include <locale>
#include "resource.h"
void DeleteLogFile()
{
	//DeleteFile(LOGFILENAME);
}

CDialog* m_globalDlg;
#define IDC_STATUS                      2500

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
