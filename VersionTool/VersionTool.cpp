#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#define VER_FILE "res\\version.h"

void replaceVersionLine(std::string& line, const std::string& macro, int newValue)
{
    if (line.find(macro) != std::string::npos)
    {
        line = "#define " + macro + " " + std::to_string(newValue);
    }
}

int main()
{
    std::cout << "Secured Globe Version tool" << std::endl;
    int majorVersion = 1;
    int minorVersion = 0;
    int patchVersion = 0;
    int buildNumber = 0;
    bool versionFound = false;

    // Read the current version from the file
    std::ifstream versionFileIn(VER_FILE);
    if (!versionFileIn.is_open())
    {
        std::cerr << "Failed to open " << VER_FILE << " for reading." << std::endl;
        return 1;
    }

    std::string line;
    std::vector<std::string> fileContent;
    while (std::getline(versionFileIn, line))
    {
        if (line.find("#define VERSION_MAJOR") != std::string::npos)
        {
            majorVersion = std::stoi(line.substr(line.find_last_of(" ") + 1));
            versionFound = true;
        }
        else if (line.find("#define VERSION_MINOR") != std::string::npos)
        {
            minorVersion = std::stoi(line.substr(line.find_last_of(" ") + 1));
            versionFound = true;
        }
        else if (line.find("#define VERSION_PATCH") != std::string::npos)
        {
            patchVersion = std::stoi(line.substr(line.find_last_of(" ") + 1));
            versionFound = true;
        }
        else if (line.find("#define VERSION_BUILD") != std::string::npos)
        {
            buildNumber = std::stoi(line.substr(line.find_last_of(" ") + 1));
            versionFound = true;
        }
        fileContent.push_back(line);
    }
    versionFileIn.close();

    if (!versionFound)
    {
        std::cerr << "Version macros not found in " << VER_FILE << std::endl;
        return 1;
    }

    // Increment the build number
    ++buildNumber;

    // Update the version lines in the content
    for (std::string& line : fileContent)
    {
        replaceVersionLine(line, "VERSION_MAJOR", majorVersion);
        replaceVersionLine(line, "VERSION_MINOR", minorVersion);
        replaceVersionLine(line, "VERSION_PATCH", patchVersion);
        replaceVersionLine(line, "VERSION_BUILD", buildNumber);
    }

    // Ensure STRINGIZE macros and version strings are preserved
    bool stringizeFound = false;
    bool stringize2Found = false;
    bool fileVersionStringFound = false;
    bool productVersionStringFound = false;

    for (const auto& line : fileContent)
    {
        if (line.find("#define STRINGIZE2") != std::string::npos)
        {
            stringize2Found = true;
        }
        if (line.find("#define STRINGIZE") != std::string::npos)
        {
            stringizeFound = true;
        }
        if (line.find("#define FILE_VERSION_STRING") != std::string::npos)
        {
            fileVersionStringFound = true;
        }
        if (line.find("#define PRODUCT_VERSION_STRING") != std::string::npos)
        {
            productVersionStringFound = true;
        }
    }

    if (!stringize2Found)
    {
        fileContent.push_back("#define STRINGIZE2(s) #s");
    }
    if (!stringizeFound)
    {
        fileContent.push_back("#define STRINGIZE(s) STRINGIZE2(s)");
    }
    if (!fileVersionStringFound)
    {
        fileContent.push_back("#define FILE_VERSION_STRING STRINGIZE(VERSION_MAJOR) \".\" STRINGIZE(VERSION_MINOR) \".\" STRINGIZE(VERSION_PATCH) \".\" STRINGIZE(VERSION_BUILD)");
    }
    if (!productVersionStringFound)
    {
        fileContent.push_back("#define PRODUCT_VERSION_STRING FILE_VERSION_STRING");
    }

    // Write the updated content back to the file
    std::ofstream versionFileOut(VER_FILE);
    if (!versionFileOut.is_open())
    {
        std::cerr << "Failed to open " << VER_FILE << " for writing." << std::endl;
        return 1;
    }

    for (const std::string& line : fileContent)
    {
        versionFileOut << line << "\n";
    }

    versionFileOut.close();
    std::cout << "Version file updated to " << majorVersion << "." << minorVersion << "." << patchVersion << "." << buildNumber << std::endl;

    return 0;
}
