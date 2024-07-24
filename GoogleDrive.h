
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
#include <curl/curl.h>
#include <fstream>
#include <unordered_map>
#include <cstdio>
#include <typeinfo>
#include <stdlib.h>



int isValidGoogleDriveOrDocsURL(const std::string& url);
std::string convertToDownloadableURL(const std::string& url);
std::string getFinalURL(const std::string& url);
std::string fetchHTMLContent(const std::string& url);
std::string constructFinalURL(const std::string& htmlContent, const std::string& baseUrl);
bool handleHTMLFile(const std::string& url, std::string& finalDownloadUrl);
bool isHTMLContent(const std::string& url);
size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* stream);
int ProgressCallback(void* ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded);
bool downloadFile(const std::string& url, const std::string& localPath);
size_t header_callbacker(char* buffer, size_t size, size_t nitems, std::string* userdata);
std::string getFileName(const std::string& url);
std::string gen_random(const int len);
CString GetFullPath(const std::string& relativePath);
CString processGoogleDrive(CString& url);
