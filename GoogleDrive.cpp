#include "pch.h"
#include "utils.h"
#include <curl\curl.h>

#include "GoogleDrive.h"


int isValidGoogleDriveOrDocsURL(const std::string& url)
{
	std::regex driveRegex(R"(https://drive\.google\.com/.*)");
	std::regex userContentRegex(R"(https://drive\.usercontent\.google\.com/.*)");
	std::regex docsRegex(R"(https://docs\.google\.com/document/d/([a-zA-Z0-9_-]+)/.*)");

	if (std::regex_match(url, userContentRegex)) return 1; // User content URL
	if (std::regex_match(url, driveRegex)) return 2; // Google Drive URL
	if (std::regex_match(url, docsRegex)) return 3; // Google Docs URL

	return 0; // Invalid URL
}

std::string convertToDownloadableURL(const std::string& url)
{
	std::regex driveRegex(R"(https://drive\.google\.com/file/d/([a-zA-Z0-9_-]+)/view.*)");
	std::regex userContentRegex(R"(https://drive\.usercontent\.google\.com/download\?id=([a-zA-Z0-9_-]+)&.*)");
	std::regex docsRegex(R"(https://docs\.google\.com/document/d/([a-zA-Z0-9_-]+)/.*)");
	std::smatch match;

	if (std::regex_match(url, match, driveRegex) && match.size() == 2)
	{
		std::string fileId = match[1].str();
		WriteLogFile(L"+ URL is regular drive url : https://drive.google.com/uc?export=download&id=%S\n", fileId.c_str());
		return "https://drive.google.com/uc?export=download&id=" + fileId;
	}

	if (std::regex_match(url, match, userContentRegex) && match.size() == 2)
	{
		std::string fileId = match[1].str();
		WriteLogFile(L"+ URL is usercontent url : https://drive.usercontent.google.com/download?id=%S\n", fileId.c_str());
		return "https://drive.usercontent.google.com/download?id=" + fileId;
	}

	if (std::regex_match(url, match, docsRegex) && match.size() == 2)
	{
		std::string fileId = match[1].str();
		WriteLogFile(L"+ URL is Google Docs url : https://docs.google.com/uc?export=download&confirm=1&id=%S\n", fileId.c_str());
		return "https://docs.google.com/uc?export=download&confirm=1&id=" + fileId;
	}

	return "";
}

std::string getFinalURL(const std::string& url)
{
	CURLcode res;
	char* effectiveUrl = nullptr;
	CURL* curl;
	curl = curl_easy_init();
	if (!curl)
	{
		WriteLogFile(L"- curl_easy_init() failed");
		return "";
	}

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects

	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L); // Maximum number of redirects to follow

	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L); // Set timeout duration to one minute

	curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
	res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		WriteLogFile(L"- curl_easy_perform() failed: %S",curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		return "";
	}
	long responseCode;
	res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
	if (res == CURLE_OK)
	{
		if (responseCode != 200)
		{
			WriteLogFile(L"- Response Code is Wrong : %d",responseCode);
			return "";
		}

	}
	else
	{
		WriteLogFile(L"- curl_easy_getinfo() failed to get response code: %S",curl_easy_strerror(res));
		return "";
	}
	char* tmpUrl;

	res = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &tmpUrl);
	if ((res == CURLE_OK) && tmpUrl)
	{
		effectiveUrl = tmpUrl;
		WriteLogFile(L"+ Next Stage URL : %S",effectiveUrl);
	}
	else
	{
		WriteLogFile(L"- curl_easy_getinfo() failed: %S",curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		return "";
	}

	//curl_easy_cleanup(curl);


	return effectiveUrl;
}

std::string fetchHTMLContent(const std::string& url)
{
	//std::cout << "+ fetchHTMLContent is called." << std::endl;

	HINTERNET hInternet = InternetOpen(L"MyUserAgent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hInternet)
	{
		std::cerr << "- InternetOpen() failed, error: " << GetLastError() << std::endl;
		return "";
	}
	//std::cout << "+ hInternet Created." << std::endl;

	HINTERNET hUrl = InternetOpenUrlW(hInternet, std::wstring(url.begin(), url.end()).c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (!hUrl)
	{
		std::cerr << "- InternetOpenUrl() failed, error: " << GetLastError() << std::endl;
		InternetCloseHandle(hInternet);
		return "";
	}
	//std::cout << "+ hUrl created" << std::endl;

	std::stringstream ss;
	char buffer[2048];
	DWORD bytesRead;
	while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
	{
		ss.write(buffer, bytesRead);
	}
	//std::cout << "+ The content of code : "<<ss.str() << std::endl;

	InternetCloseHandle(hUrl);
	InternetCloseHandle(hInternet);

	return ss.str();
}

std::string constructFinalURL(const std::string& htmlContent, const std::string& baseUrl)
{
	//std::cout << "constructFinalURL called" << std::endl;

	std::regex idRegex("(<input type=\"hidden\" name=\"id\" value=\"([^\"]+)\">)");
	std::regex confirmRegex("(<input type=\"hidden\" name=\"confirm\" value=\"([^\"]+)\">)");
	std::regex uuidRegex("(<input type=\"hidden\" name=\"uuid\" value=\"([^\"]+)\">)");

	std::smatch match;
	std::string id, confirm, uuid;

	if (std::regex_search(htmlContent, match, idRegex) && match.size() == 3)
	{
		id = match[2].str();
		printf("+ Extracted file ID : %s\n", id.c_str());
	}
	if (std::regex_search(htmlContent, match, confirmRegex) && match.size() == 3)
	{
		confirm = match[2].str();
		printf("+ Confirm Method : %s\n", confirm.c_str());

	}
	if (std::regex_search(htmlContent, match, uuidRegex) && match.size() == 3)
	{
		uuid = match[2].str();
		printf("+ UUID : %s\n", uuid.c_str());
	}

	if (!id.empty() && !confirm.empty() && !uuid.empty())
	{
		printf("+ Next Stage URL : %s\n", (baseUrl + "?id=" + id + "&confirm=" + confirm + "&uuid=" + uuid).c_str());
		return baseUrl + "?id=" + id + "&confirm=" + confirm + "&uuid=" + uuid;
	}

	return "";
}

bool handleHTMLFile(const std::string& url, std::string& finalDownloadUrl)
{
	std::string htmlContent = fetchHTMLContent(url);
	if (htmlContent.find("id=\"uc-download-link\"") != std::string::npos)
	{
		//printf("+ Found HTML content in the URL\n");
		finalDownloadUrl = constructFinalURL(htmlContent, "https://drive.usercontent.google.com/download");
		return !finalDownloadUrl.empty();
	}
	return false;
}

bool isHTMLContent(const std::string& url)
{
	//std::cout << "+ isHTMLContent Called." << std::endl;

	CURL* curl;
	CURLcode res;
	char* contentType = nullptr;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if (!curl)
	{
		std::cerr << "- curl_easy_init() failed" << std::endl;
		return false;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // We only want the headers
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L); // Set timeout duration to one minute

	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		std::cerr << "- curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		return false;
	}

	res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contentType);
	if (res != CURLE_OK || !contentType)
	{
		std::cerr << "- curl_easy_getinfo() failed: " << curl_easy_strerror(res) << std::endl;
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		return false;
	}

	std::string contentTypeStr(contentType);
	//std::cout << "+ The contentType is : " << contentTypeStr << std::endl;

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return contentTypeStr.find("html") != std::string::npos;
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
		std::cout << "\rDownload Progress: " << percentage << "% (" << nowDownloaded << " / " << totalToDownload << " bytes)" << std::flush;
	}
	return 0; // Return 0 to continue the download
}
bool downloadFile(const std::string& url, const std::string& localPath)
{

	printf("+ Downloading URL : %s \n + The Location : %s\n", url.c_str(), localPath.c_str());


	CURL* curl;
	CURLcode res;
	std::ofstream outFile(localPath, std::ios::binary);


	if (!outFile.is_open())
	{
		std::cerr << "- Failed to open file: " << localPath << std::endl;
		return false;
	}

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if (!curl)
	{
		std::cerr << "- curl_easy_init() failed" << std::endl;
		return false;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
	//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L); // Set timeout duration to one minute
	curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback); // Set progress callback function
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L); // Enable progress meter

	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		std::cerr << "- curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		return false;
	}

	curl_easy_cleanup(curl);
	curl_global_cleanup();
	outFile.close();


	std::cout << "\n+ Download completed!" << std::endl;
	return true;
}


size_t header_callbacker(char* buffer, size_t size, size_t nitems, std::string* userdata)
{
	std::string header(buffer, size * nitems);
	const std::string filename_header = "Content-Disposition: attachment; filename=";
	if (header.find(filename_header) != std::string::npos)
	{
		size_t pos = header.find(filename_header) + filename_header.size();
		size_t end_pos = header.find("\r\n", pos);
		*userdata = header.substr(pos, end_pos - pos);

		// Remove quotes from the filename
		userdata->erase(std::remove(userdata->begin(), userdata->end(), '\"'), userdata->end());
	}
	return nitems * size;
}


std::string getFileName(const std::string& url)
{
	CURL* curl;
	CURLcode res;
	std::string file_name;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callbacker);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, &file_name);
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			fprintf(stderr, "- curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			return "";
		}
	}
	return file_name;

}

std::string gen_random(const int len)
{
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	std::string tmp_s;
	tmp_s.reserve(len);

	for (int i = 0; i < len; ++i)
	{
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	return tmp_s;
}

CString GetFullPath(const std::string& relativePath)
{
	// Convert std::string to std::wstring
	int wideCharSize = MultiByteToWideChar(CP_ACP, 0, relativePath.c_str(), -1, NULL, 0);
	std::wstring wideRelativePath(wideCharSize, 0);
	MultiByteToWideChar(CP_ACP, 0, relativePath.c_str(), -1, &wideRelativePath[0], wideCharSize);

	wchar_t fullPath[MAX_PATH];
	if (GetFullPathNameW(wideRelativePath.c_str(), MAX_PATH, fullPath, NULL) == 0)
	{
		// Handle the error, GetLastError() can be used to get more error details
		WriteLogFile(L"- Error getting full path: %d",GetLastError());
		return L"";
	}
	return CString(fullPath);
}

CString processGoogleDrive(CString& url)
{
	CT2A c_url(url);
	std::string s_url(c_url);
	int urlType = isValidGoogleDriveOrDocsURL(s_url);
	if (urlType == 0)
	{
		WriteLogFile(L"- The URL is not a valid Google Drive, or user content URL.");
		return CString();
	}
	std::string downloadUrl = convertToDownloadableURL(s_url);
	if (downloadUrl.empty())
	{
		WriteLogFile(L"- The URL is not a valid Google Drive file URL.");
		return CString();
	}

	std::string finalDownloadUrl;
	if (urlType == 1)
	{
		std::string htmlContent = fetchHTMLContent(downloadUrl);
		finalDownloadUrl = constructFinalURL(htmlContent, "https://drive.usercontent.google.com/download");
	}
	else
	{
		std::string finalUrl = getFinalURL(downloadUrl);
		WriteLogFile(L"+ the finalURL : %S",finalUrl.c_str());
		

		if (finalUrl.empty())
		{
			WriteLogFile(L"- Failed to get the final URL.");
			return CString();
		}


		if (isHTMLContent(finalUrl))
		{
			if (!handleHTMLFile(finalUrl, finalDownloadUrl))
			{
				WriteLogFile(L"- Failed to construct final download URL from HTML content.");
				return CString();
			}
		}
		else
		{
			finalDownloadUrl = finalUrl;


		}
	}

	std::string localPath;
	std::string filename = getFileName(finalDownloadUrl);
	if (!filename.empty())
	{
		localPath = filename;
	}
	else
	{
		localPath = gen_random(7) + ".bin";
	}



	if (downloadFile(finalDownloadUrl, localPath))
	{

		WriteLogFile(L"+ The file has been downloaded successfully to %S",localPath.c_str());
	}	
	else
	{
		WriteLogFile(L"- Failed to download the file.");
		return CString();
	}
	CString fullPath = GetFullPath(localPath);
	if (!fullPath.IsEmpty())
	{
		WriteLogFile(L"- Full path : %s",fullPath.GetString());
		return fullPath;

	}
	else
	{
		WriteLogFile(L"- Failed to get full path.");
		return CString();
	}

}
