
#include "pch.h"
#include "utils.h"
#include <curl\curl.h>

#include "GoogleDrive.h"

// std::wostringstream conv;


int isValidGoogleDriveOrDocsURL(const std::wstring& url)
{
    std::wregex driveRegex(LR"(https://drive\.google\.com/.*)");
    std::wregex userContentRegex(LR"(https://drive\.usercontent\.google\.com/.*)");
    std::wregex docsRegex(LR"(https://docs\.google\.com/document/d/([a-zA-Z0-9_-]+)/.*)");

    if (std::regex_match(url, userContentRegex)) return 1; // User content URL
    if (std::regex_match(url, driveRegex)) return 2; // Google Drive URL
    if (std::regex_match(url, docsRegex)) return 3; // Google Docs URL

    return 0; // Invalid URL
}


std::wstring convertToDownloadableURL(const std::wstring& url)
{
    std::wregex driveRegex(LR"(https://drive\.google\.com/file/d/([a-zA-Z0-9_-]+)/view.*)");
    std::wregex userContentRegex(LR"(https://drive\.usercontent\.google\.com/download\?id=([a-zA-Z0-9_-]+)&.*)");
    std::wregex docsRegex(LR"(https://docs\.google\.com/document/d/([a-zA-Z0-9_-]+)/.*)");
    std::wsmatch match;

    if (std::regex_match(url, match, driveRegex) && match.size() == 2)
    {
        std::wstring fileId = match[1].str();
        WriteLogFile(L"+ URL is regular drive url : https://drive.google.com/uc?export=download&id=%s", fileId.c_str());
        return L"https://drive.google.com/uc?export=download&id=" + fileId;
    }

    if (std::regex_match(url, match, userContentRegex) && match.size() == 2)
    {
        std::wstring fileId = match[1].str();
        WriteLogFile(L"+ URL is usercontent url : https://drive.usercontent.google.com/download?id=%s", fileId.c_str());
        return L"https://drive.usercontent.google.com/download?id=" + fileId;
    }

    if (std::regex_match(url, match, docsRegex) && match.size() == 2)
    {
        std::wstring fileId = match[1].str();
        WriteLogFile(L"+ URL is Google Docs url : https://docs.google.com/uc?export=download&confirm=1&id=%s", fileId.c_str());
        return L"https://docs.google.com/uc?export=download&confirm=1&id=" + fileId;
    }

    return L"";
}
std::wstring stringToWString(const std::string& str)
{
    // size_t len = mbstowcs(nullptr, str.c_str(), 0);
    // std::wstring wstr(len, L'\0');
    // mbstowcs(&wstr[0], str.c_str(), len);
    // conv << str.c_str();
    // std::wstring wstr(conv.str());
    std::wstring wstr(str.begin(), str.end());
    return wstr;
}

// Convert std::wstring to std::string
std::string wstringToString(const std::wstring& wstr)
{
    /*size_t len = wcstombs(nullptr, wstr.c_str(), 0);
    std::string str(len, '\0');
    wcstombs(&str[0], wstr.c_str(), len);*/
    std::string str(wstr.begin(), wstr.end());
    return str;
}
std::wstring getFinalURL(const std::wstring& url)
{
    CURLcode res;
    //wchar_t* effectiveUrl = nullptr;
    std::string effectiveUrl;
    CURL* curl = curl_easy_init();

    if (!curl)
    {
        WriteLogFile(L"- curl_easy_init() failed in getFinalURL");
        return L"";
    }

    // Convert std::wstring to std::string
    //std::string url_str(url.begin(), url.end());
    std::string url_str = wstringToString(url);

    curl_easy_setopt(curl, CURLOPT_URL, url_str.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L); // Maximum number of redirects to follow
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L); // Set timeout duration to one minute
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        WriteLogFile(L"- curl_easy_perform() failed: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return L"";
    }


    long responseCode;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);


    if (res == CURLE_OK)
    {
        if (responseCode != 200)
        {
            WriteLogFile(L"- Response Code is Wrong: %ld", +responseCode);
            curl_easy_cleanup(curl);
            return L"";
        }
    }
    else
    {
        WriteLogFile(L"- curl_easy_getinfo() failed to get response code: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return L"";
    }

    char* tmpUrl;
    res = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &tmpUrl);


    if ((res == CURLE_OK) && tmpUrl)
    {
        //std::string tmpUrl_str(tmpUrl);
        /*std::wstring tmpUrl_wstr(tmpUrl_str.begin(), tmpUrl_str.end());
        effectiveUrl = new wchar_t[tmpUrl_wstr.length() + 1];*/
        //std::wcscpy(effectiveUrl, tmpUrl_wstr.c_str());
        std::string effectiveUrl(tmpUrl);

        std::wstring finalUrl = stringToWString(effectiveUrl);
        WriteLogFile(L"+ Next Stage URL: %s", finalUrl.c_str());

        curl_easy_cleanup(curl);

        return finalUrl;

    }
    else
    {
        WriteLogFile(L"- curl_easy_getinfo() failed: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return L"";
    }
    // _setmode(_fileno(stdout), _O_WTEXT);


    //std::wstring finalUrl(effectiveUrl);
    //delete[] effectiveUrl;
}


std::wstring fetchHTMLContent(const std::wstring& url)
{
    //WriteLogFile(L"+ fetchHTMLContent is called.");

    HINTERNET hInternet = InternetOpen(L"MyUserAgent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet)
    {
        WriteLogFile(L"- InternetOpen() failed, error: %s", GetLastError());
        return L"";
    }
    //WriteLogFile(L"+ hInternet Created.");

    HINTERNET hUrl = InternetOpenUrlW(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl)
    {
        WriteLogFile(L"- InternetOpenUrl() failed, error: %s", GetLastError());
        InternetCloseHandle(hInternet);
        return L"";
    }
    //WriteLogFile(L"+ hUrl created");

    std::wstringstream ss;
    wchar_t buffer[2048];
    DWORD bytesRead;
    while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
    {
        ss.write(buffer, bytesRead / sizeof(wchar_t)); // Adjust for wide character size
    }
    WriteLogFile(L"+ The content of the fetched HTML: %s", ss.str());

    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);

    return ss.str();
}

std::wstring constructFinalURL(const std::wstring& htmlContent, const std::wstring& baseUrl)
{
    //WriteLogFile(L"constructFinalURL called");

    std::wregex idRegex(L"(<input type=\"hidden\" name=\"id\" value=\"([^\"]+)\" > )");
    std::wregex confirmRegex(L"(<input type=\"hidden\" name=\"confirm\" value=\"([^\"]+)\" > )");
    std::wregex uuidRegex(L"(<input type=\"hidden\" name=\"uuid\" value=\"([^\"]+)\" > )");

    std::wsmatch match;
    std::wstring id, confirm, uuid;

    if (std::regex_search(htmlContent, match, idRegex) && match.size() == 2)
    {
        id = match[1].str();
        WriteLogFile(L"+ Extracted file ID: %s", id.c_str());
    }
    if (std::regex_search(htmlContent, match, confirmRegex) && match.size() == 2)
    {
        confirm = match[1].str();
        WriteLogFile(L"+ Confirm Method: %s", confirm.c_str());
    }
    if (std::regex_search(htmlContent, match, uuidRegex) && match.size() == 2)
    {
        uuid = match[1].str();
        WriteLogFile(L"+ UUID: %s", uuid.c_str());
    }

    if (!id.empty() && !confirm.empty() && !uuid.empty())
    {
        std::wstring finalUrl = baseUrl + L"?id=" + id + L"&confirm=" + confirm + L"&uuid=" + uuid;
        WriteLogFile(L"+ Next Stage URL: %s", finalUrl.c_str());
        return finalUrl;
    }

    return L"";
}

bool handleHTMLFile(const std::wstring& url, std::wstring& finalDownloadUrl)
{
    std::wstring htmlContent = fetchHTMLContent(url);
    if (htmlContent.find(L"id=\"uc-download-link\"") != std::wstring::npos)
    {
        WriteLogFile(L"+ Found HTML content in the URL");
        finalDownloadUrl = constructFinalURL(htmlContent, L"https://drive.usercontent.google.com/download");
        return !finalDownloadUrl.empty();
    }
    return false;
}

bool isHTMLContent(const std::wstring& url)
{
    //WriteLogFile(L"+ isHTMLContent Called.");

    CURL* curl;
    CURLcode res;
    char* contentType = nullptr;

    //curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl)
    {
        WriteLogFile(L"- curl_easy_init() failed in isHTMLContent");
        return false;
    }

    std::string url_str(url.begin(), url.end()); // Convert std::wstring to std::string
    //WriteLogFile(L"+ The url_str : %s\n + Cstr Variant : %s",url_str,url_str.c_str());
    //std::cout<<"+ The url_str :" << url_str << typeid(url_str).name() << "\nCstr Variant : "<< url_str.c_str()<< typeid(url_str.c_str() ).name() <<"\n";

    curl_easy_setopt(curl, CURLOPT_URL, url_str.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // We only want the headers
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L); // Set timeout duration to one minute

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        WriteLogFile(L"- curl_easy_perform() failed: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        //curl_global_cleanup();
        return false;
    }

    res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contentType);
    if (res != CURLE_OK || !contentType)
    {
        WriteLogFile(L"- curl_easy_getinfo() failed: %s", +curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        //curl_global_cleanup();
        return false;
    }

    std::string contentTypeStr(contentType);
    std::wstring contentTypeWstr = stringToWString(contentTypeStr);
    WriteLogFile(L"+ The contentType is: %s", contentTypeWstr.c_str());
    //std::cout<<"+ The contentType is: %ls" << contentTypeStr<<"\n";
    //std::cout << "+ The contentType is: %ls" << contentType << "\n";



    curl_easy_cleanup(curl);
    //curl_global_cleanup();

    return contentTypeStr.find("html") != std::string::npos;
}

//size_t header_callbacker(wchar_t* buffer, size_t size, size_t nitems, std::wstring* userdata)
//{
//    std::wstring header(buffer, size * nitems);
//    std::wcout << L"The Header : " << header << L"\n";
//
//    const std::wstring filename_header = L"Content-Disposition: attachment; filename=";
//    if (header.find(filename_header) != std::wstring::npos)
//    {
//        size_t pos = header.find(filename_header) + filename_header.size();
//        size_t end_pos = header.find(L"\r\n", pos);
//        *userdata = header.substr(pos, end_pos - pos);
//
//        // Remove quotes from the filename
//        userdata->erase(std::remove(userdata->begin(), userdata->end(), L'\"'), userdata->end());
//        std::wcout<<L"The UserData : " << userdata << L"\n";
//    }
//    return nitems * size;
//}
size_t header_callbacker(char* buffer, size_t size, size_t nitems, std::string* userdata)
{
    std::string header(buffer, size * nitems);
    const std::string filename_header = "Content-Disposition: attachment; filename=";
    if (header.find(filename_header) != std::string::npos) {
        size_t pos = header.find(filename_header) + filename_header.size();
        size_t end_pos = header.find("\r\n", pos);
        *userdata = header.substr(pos, end_pos - pos);

        // Remove quotes from the filename
        userdata->erase(std::remove(userdata->begin(), userdata->end(), '\"'), userdata->end());
    }
    return nitems * size;
}
std::wstring getFileName(const std::wstring& url)
{

    CURL* curl = curl_easy_init();
    CURLcode res;
    std::string fName;

    //curl_global_init(CURL_GLOBAL_DEFAULT);
    if (curl)
    {
        //std::string url_str(url.begin(), url.end()); // Convert std::wstring to std::string
        std::string url_str = wstringToString(url);
        curl_easy_setopt(curl, CURLOPT_URL, url_str.c_str());
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callbacker);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &fName);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            WriteLogFile(L"- curl_easy_perform() failed in getFileName : %s", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            //curl_global_cleanup();
            return L"";
        }

    }
    else
    {
        WriteLogFile(L"- curl_easy_init() failed in getFileName");

    }

    if (!fName.empty())
    {
        std::wstring theFile = stringToWString(fName);
        WriteLogFile(L"+ The file Name : %s", theFile.c_str());
        return theFile;

    }
    else
    {
        WriteLogFile(L"- Failed Get FileName");


    }
    //return file_name;
    return L"";
}

std::wstring gen_random(const int len)
{
    static const wchar_t alphanum[] =
        L"0123456789"
        L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        L"abcdefghijklmnopqrstuvwxyz";
    std::wstring tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
    {
        tmp_s += alphanum[rand() % (sizeof(alphanum) / sizeof(wchar_t) - 1)];
    }

    return tmp_s;
}

std::wstring GetFullPath(const std::wstring& relativePath)
{

    wchar_t fullPath[MAX_PATH];
    if (GetFullPathNameW(relativePath.c_str(), MAX_PATH, fullPath, NULL) == 0)
    {
        // Handle the error, GetLastError() can be used to get more error details
        WriteLogFile(L"- Error getting full path : %s", GetLastError());
        return L"";
    }
    return std::wstring(fullPath);
}
size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* stream)
{
    std::ofstream* out = (std::ofstream*)stream;
    out->write((const char*)ptr, size * nmemb);
    return size * nmemb;
}
int ProgressCallback(void* ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded)
{
    if (totalToDownload != 0)
    {
        int percentage = static_cast<int>((nowDownloaded * 100) / totalToDownload);
        std::wcout << L"\rDownload Progress: " << percentage << "% (" << nowDownloaded << " / " << totalToDownload << " bytes)" << std::flush;
    }
    return 0; // Return 0 to continue the download
}
bool downloadFile(const std::wstring& url, const std::wstring& localPath) {
    WriteLogFile(L"+ Downloading URL : %ls \n + The Location : %ls\n", url.c_str(), localPath.c_str());

    CURL* curl;
    CURLcode res;
    std::ofstream outFile(localPath, std::ios::binary);

    if (!outFile.is_open()) {
        WriteLogFile(L"- Failed to open file: %s", localPath.c_str());
        return false;
    }

    //curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl)
    {
        WriteLogFile(L"- curl_easy_init() failed in downloadFile");
        return false;
    }

    std::string url_str(url.begin(), url.end()); // Convert std::wstring to std::string
    curl_easy_setopt(curl, CURLOPT_URL, url_str.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
    // curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L); // Set timeout duration to one minute
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback); // Set progress callback function
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L); // Enable progress meter

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        WriteLogFile(L"- curl_easy_perform() failed: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        //curl_global_cleanup();
        return false;
    }

    curl_easy_cleanup(curl);
    //curl_global_cleanup();
    outFile.close();

    WriteLogFile(L"\n+ Download completed!");
    return true;
}
std::wstring processGoogleDrive(std::wstring& url)
{

    int urlType = isValidGoogleDriveOrDocsURL(url);
    if (urlType == 0)
    {
        WriteLogFile(L"- The URL is not a valid Google Drive, user content URL or Google Docs url : %s",url.c_str());
        return std::wstring();
    }

    std::wstring downloadUrl = convertToDownloadableURL(url);
    if (downloadUrl.empty())
    {
        WriteLogFile(L"- The URL is not a valid Google Drive file URL : %s", url.c_str());

        return std::wstring();
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::wstring finalDownloadUrl;
    if (urlType == 1)
    {
        std::wstring htmlContent = fetchHTMLContent(downloadUrl);
        finalDownloadUrl = constructFinalURL(htmlContent, L"https://drive.usercontent.google.com/download");
    }
    else
    {
        std::wstring finalUrl = getFinalURL(downloadUrl);
        WriteLogFile(L"+ the finalURL : %s", finalUrl.c_str());


        if (finalUrl.empty())
        {
            WriteLogFile(L"- Failed to get the final URL.");
            curl_global_cleanup();

            return std::wstring();
        }


        if (isHTMLContent(finalUrl))
        {
            if (!handleHTMLFile(finalUrl, finalDownloadUrl))
            {
                WriteLogFile(L"- Failed to construct final download URL from HTML content.");
                curl_global_cleanup();

                return std::wstring();
            }
        }
        else
        {
            finalDownloadUrl = finalUrl;


        }
    }

    std::wstring localPath;

    std::wstring filename = getFileName(finalDownloadUrl);
    if (!filename.empty())
    {
        localPath = filename;
    }
    else
    {
        localPath = gen_random(7) + L".bin";
    }



    if (downloadFile(finalDownloadUrl, localPath))
    {

        WriteLogFile(L"+ The file has been downloaded successfully to %s", localPath.c_str());
    }
    else
    {
        WriteLogFile(L"- Failed to download the file.");
        curl_global_cleanup();

        return std::wstring();

    }
    std::wstring fullPath = GetFullPath(localPath);
    if (!fullPath.empty())
    {
        WriteLogFile(L"+ Full path : %s", fullPath.c_str());
        curl_global_cleanup();

        return L'\''+ fullPath+ L'\'';

    }
    else
    {
        WriteLogFile(L"- Failed to get full path.");
        curl_global_cleanup();

        return std::wstring();
    }

}