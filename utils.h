#include "pch.h"
#include <string>
#include <windows.h>
#include <atlbase.h>
#include <atlstr.h>

#pragma once


extern const wchar_t* DATEFORMAT;
extern CDialog* m_globalDlg;
void ShowStatus(LPCWSTR lpText);

extern wchar_t LOGFILENAME[1024];
std::string wstring_to_string(const std::wstring& wstr);
std::wstring string_to_wstring(const std::string& str);

typedef int(*logFunc)(LPCWSTR lpText, ...);

CAtlString CurrentDate();
std::wstring CurrentTimeForUpdate();
void ShowError(HRESULT errorCode);
void OpenLogFile();
void DeleteLogFile();
std::wstring removeSubstring(const std::wstring& original, const std::wstring& substringToRemove);
int WriteLogFile(LPCWSTR lpText, ...);
