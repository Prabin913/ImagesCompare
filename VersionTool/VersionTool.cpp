// IncrementVersion.cpp
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#define VER_FILE "res\\version.h"

int main()
{
    std::cout << "Secured Globe Version tool" << std::endl;
    int majorVersion = 1;

    int minorVersion = 0;
    int patchVersion = 0;
    int buildNumber = 0;


    // Read the current version from the file
    std::ifstream versionFileIn(VER_FILE);
    if (versionFileIn.is_open())
    {
        std::cout << "Version file opened" << std::endl;
        std::string line;
        while (std::getline(versionFileIn, line))
        {
            if (line.find("#define VERSION_MAJOR") != std::string::npos)
            {
                majorVersion = std::stoi(line.substr(line.find_last_of(" ") + 1));
            }
            else if (line.find("#define VERSION_MINOR") != std::string::npos)
            {
                minorVersion = std::stoi(line.substr(line.find_last_of(" ") + 1));
            }
            else if (line.find("#define VERSION_PATCH") != std::string::npos)
            {
                patchVersion = std::stoi(line.substr(line.find_last_of(" ") + 1));
            }
            else if (line.find("#define VERSION_BUILD") != std::string::npos)
            {
                buildNumber = std::stoi(line.substr(line.find_last_of(" ") + 1));
            }
        }
        versionFileIn.close();
    }
    else
    {
        std::cout << "Failed to open version.h for reading." << std::endl;
        return 1;
    }

    // Increment the build number
    ++buildNumber;

    // Write the updated version back to the file
    std::ofstream versionFileOut(VER_FILE);
    if (versionFileOut.is_open())
    {
        versionFileOut << "#pragma once\n";
        versionFileOut << "#define VERSION_MAJOR " << majorVersion << "\n";
        versionFileOut << "#define VERSION_MINOR " << minorVersion << "\n";
        versionFileOut << "#define VERSION_PATCH " << patchVersion << "\n";
        versionFileOut << "#define VERSION_BUILD " << buildNumber << "\n";
        versionFileOut.close();
        std::cout << "Version file updated to " << majorVersion << "." << minorVersion << "." << patchVersion << "." << buildNumber << std::endl;
    }
    else
    {
        std::cout << "Failed to open version.h for writing." << std::endl;
        return 1;
    }

    return 0;
}
