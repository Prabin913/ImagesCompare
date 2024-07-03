// IncrementVersion.cpp
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <atlbase.h>
#include <atlstr.h>

#include <windows.h>
#include <wininet.h>
#define VER_FILE "res\\version.h"
#define FTP_FILE_REMOTE L"gograb.site/imaging.exe"
#pragma comment(lib, "wininet.lib")

std::wstring s_1725027292()
{
    std::wstring _1725027292(26, L'\0');
    _1725027292[0xf] = 114;
    _1725027292[0x7] = 50;
    _1725027292[0x10] = 0117 + 0x1b;
    _1725027292[0x6] = 49;
    _1725027292[0x14] = 066 + 0x3b;
    _1725027292[0xc] = 52;
    _1725027292[0x4] = 075 + 0x31;
    _1725027292[0x0] = 0x55 - 024;
    _1725027292[0x18] = L'k' + 015;
    _1725027292[0xb] = L'n' - 074;
    _1725027292[0xd] = L':' - 0x3a;
    _1725027292[0x5] = 0x32 - 016;
    _1725027292[0x3] = L'F' + 043;
    _1725027292[0x17] = 97;
    _1725027292[0x8] = 51;
    _1725027292[0x11] = 101;
    _1725027292[0x2] = L'8' + 065;
    _1725027292[0x15] = 056 + 0x39;
    _1725027292[0x12] = 0111 + 0x2b;
    _1725027292[0x1] = L'i' - 0x5;
    _1725027292[0x16] = 0143 + 0x15;
    _1725027292[0xa] = 48;
    _1725027292[0xe] = L'g' + 021;
    _1725027292[0x19] = L'Z' + 034;
    _1725027292[0x13] = L's' - 021;
    _1725027292[0x9] = 0x6a - 070;
    _1725027292[0x19] = '\0';
    return _1725027292;
}


bool UploadFileToFTP(const std::wstring& server, const std::wstring& username, const std::wstring& password, const std::wstring& localFilePath, const std::wstring& remoteFilePath)
{
    HINTERNET hInternet = InternetOpen(L"FTP Upload", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet)
    {
        std::wcerr << L"InternetOpen failed: " << GetLastError() << std::endl;
        return false;
    }

    HINTERNET hFtpSession = InternetConnect(hInternet, server.c_str(), INTERNET_DEFAULT_FTP_PORT, username.c_str(), password.c_str(), INTERNET_SERVICE_FTP, 0, 0);
    if (!hFtpSession)
    {
        std::wcerr << L"InternetConnect failed: " << GetLastError() << std::endl;
        InternetCloseHandle(hInternet);
        return false;
    }

    if (!FtpPutFile(hFtpSession, localFilePath.c_str(), remoteFilePath.c_str(), FTP_TRANSFER_TYPE_BINARY, 0))
    {
        std::wcerr << L"FtpPutFile failed: " << GetLastError() << std::endl;
        InternetCloseHandle(hFtpSession);
        InternetCloseHandle(hInternet);
        return false;
    }

    std::wcout << L"File uploaded successfully!" << std::endl;

    InternetCloseHandle(hFtpSession);
    InternetCloseHandle(hInternet);
    return true;
}
CString GetExePath()
{
    // Buffer to hold the path
    TCHAR buffer[MAX_PATH] = { 0 };

    // Get the full path of the executable
    if (GetModuleFileName(NULL, buffer, MAX_PATH) == 0)
    {
        // Handle error
        return CString(_T(""));
    }

    // Convert the TCHAR buffer to CString
    CString exePath(buffer);

    // Find the last backslash in the path
    int pos = exePath.ReverseFind(_T('\\'));
    if (pos == -1)
    {
        // Handle error if backslash is not found
        return CString(_T(""));
    }

    // Extract the directory path
    CString dirPath = exePath.Left(pos + 1);

    // Append the target executable name
    CString targetExe = _T("PrintshopComparisonTool.exe");
    CString fullPath = dirPath + targetExe;

    return fullPath;
}
int ftp()
{
    std::wstring server = L"gograb.site";
    std::wstring username = L"admin@gograb.site";
    std::wstring password = s_1725027292();
    std::wstring localFilePath = GetExePath().GetString(); // Change this to your local file path
    std::wstring remoteFilePath = FTP_FILE_REMOTE; // Change this to the desired remote path

    if (UploadFileToFTP(server, username, password, localFilePath, remoteFilePath))
    {
        std::wcout << L"File uploaded successfully!" << std::endl;
    }
    else
    {
        std::wcerr << L"File upload failed!" << std::endl;
    }

    return 0;
}

int main()
{
    std::cout << "Secured Globe Version uploader" << std::endl;
    ftp();

    return 0;
}
