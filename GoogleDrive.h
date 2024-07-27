
//+++++++++++++++++++++++++++++++++++Ismayil Code +++++++++++++++++++++++++++++++++++
//#define CURL_STATICLIB
//Crypt32.lib; Normaliz.lib; Wldap32.lib; Ws2_32.lib

#include <iostream>
#include <regex>
#include <string>
#include <windows.h>
#include <wininet.h>
#include <urlmon.h>
#include <cstdlib> // For malloc and free
#include <sstream>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "libcurl_a.lib")

#include <curl/curl.h>
#include <fstream>
#include <unordered_map>
#include <cstdio>
#include <typeinfo>
#include <stdlib.h>
#include <codecvt>
#include <fcntl.h>
#include <io.h>




int isValidGoogleDriveOrDocsURL(const std::wstring& url);
std::wstring convertToDownloadableURL(const std::wstring& url);
std::wstring getFinalURL(const std::wstring& url);
std::wstring fetchHTMLContent(const std::wstring& url);
std::wstring constructFinalURL(const std::wstring& htmlContent, const std::wstring& baseUrl);
bool handleHTMLFile(const std::wstring& url, std::wstring& finalDownloadUrl);
bool isHTMLContent(const std::wstring& url);
size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* stream);
int ProgressCallback(void* ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded);
bool downloadFile(const std::wstring& url, const std::wstring& localPath);
//size_t header_callbacker(wchar_t* buffer, size_t size, size_t nitems, std::wstring* userdata);
size_t header_callbacker(char* buffer, size_t size, size_t nitems, std::string* userdata);
std::wstring getFileName(const std::wstring& url);
std::wstring gen_random(const int len);
std::wstring GetFullPath(const std::wstring& relativePath);
std::wstring processGoogleDrive(std::wstring& url);
