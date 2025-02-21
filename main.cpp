#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <cstdlib>

namespace fs = std::filesystem;

// ANSI escape codes for colors
#define COLOR_RESET   "\033[0m"    // Reset to default color
#define COLOR_RED     "\033[31m"   // Red color
#define COLOR_GREEN   "\033[32m"   // Green color
#define COLOR_DARK_BLUE "\033[34m" // Dark blue color

// Function to get the current date in UK format (DD-MM-YYYY)
std::string getCurrentDate() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    char buffer[11];
    std::strftime(buffer, sizeof(buffer), "%d-%m-%Y", now);
    return std::string(buffer);
}

// Function to split input string into a vector of package names
std::vector<std::string> splitInput(const std::string &input, const std::string &delimiter = " ") {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string inputCopy = input;
    while ((pos = inputCopy.find(delimiter)) != std::string::npos) {
        std::string token = inputCopy.substr(0, pos);
        if (!token.empty()) {
            tokens.push_back(token);
        }
        inputCopy.erase(0, pos + delimiter.length());
    }
    if (!inputCopy.empty()) {
        tokens.push_back(inputCopy);
    }
    return tokens;
}

// Function to execute system commands directly
void executeCommand(const std::string &command) {
    int result = std::system(command.c_str());

    if (result != 0) {
        std::cerr << COLOR_RED << "Error executing command: " << command << COLOR_RESET << std::endl;
    }
}

// Function to download and store .pkg.tar.zst packages
void downloadAndStorePackage(const std::vector<std::string> &packageNames, const std::string &dateFolder) {
    for (const auto &packageName : packageNames) {
        std::string packageDir = "packages/" + dateFolder + "/" + packageName;

        // Create directory for the package
        std::string createDirCommand = "mkdir -p " + packageDir;
        executeCommand(createDirCommand);

        std::cout << COLOR_DARK_BLUE << "Downloading package: " << packageName << COLOR_RESET << std::endl;

        // Download the package
        std::string downloadCommand = "sudo pacman -Sw --noconfirm " + packageName;
        executeCommand(downloadCommand);

        // Move the downloaded package to the target directory
        std::string moveCommand = "sudo mv /var/cache/pacman/pkg/" + packageName + "-*.pkg.tar.zst " + packageDir + "/";
        executeCommand(moveCommand);

        std::cout << COLOR_GREEN << "Package " << packageName << " downloaded successfully to " << packageDir << COLOR_RESET << std::endl;
    }
}

// Function to create SquashFS for a specific package
void createSquashFSForPackage(const std::string &packageName, const std::string &dateFolder) {
    std::string packageDir = "packages/" + dateFolder + "/" + packageName;
    std::string squashfsFile = packageDir + ".sfs";

    std::ostringstream oss;
    oss << "sudo mksquashfs " << packageDir << " " << squashfsFile
    << " -comp xz -b 1M -no-duplicates -no-recovery -always-use-fragments -wildcards -xattrs"
    << " -Xdict-size 100% -Xbcj x86";

    std::cout << COLOR_DARK_BLUE << "Creating SquashFS archive for " << packageName << "..." << COLOR_RESET << std::endl;
    executeCommand(oss.str());
    std::cout << COLOR_GREEN << "SquashFS archive created successfully: " << squashfsFile << COLOR_RESET << std::endl;
}

// Function to create SquashFS for the entire packages folder
void createSquashFSForPackagesFolder() {
    std::string squashfsFile = "packages.sfs";

    std::ostringstream oss;
    oss << "sudo mksquashfs packages " << squashfsFile
    << " -comp xz -b 1M -no-duplicates -no-recovery -always-use-fragments -wildcards -xattrs"
    << " -Xdict-size 100% -Xbcj x86";

    std::cout << COLOR_DARK_BLUE << "Creating SquashFS archive for the 'packages' folder..." << COLOR_RESET << std::endl;
    executeCommand(oss.str());
    std::cout << COLOR_GREEN << "SquashFS archive created successfully: " << squashfsFile << COLOR_RESET << std::endl;
}

// Function to store package names in the log file
void storePackageNamesInLog(const std::vector<std::string> &packageNames) {
    std::string currentDate = getCurrentDate();

    // Open the download log file for logging
    std::string logFileName = "download-" + currentDate + ".txt";
    std::ofstream logFile(logFileName, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << COLOR_DARK_BLUE << "Error opening download log file." << COLOR_RESET << std::endl;
        return;
    }

    // Log the purpose of the file
    logFile << "Download Log - " << currentDate << "\n";

    // Log all package names
    for (const auto &packageName : packageNames) {
        logFile << "Package \"" << packageName << "\"\n";
        std::cout << COLOR_GREEN << "Package " << packageName << " logged successfully." << COLOR_RESET << std::endl;
    }

    logFile.close();
}

// Function to download packages from packages.txt
void downloadPackagesFromList(const std::string &dateFolder) {
    std::ifstream packagesFile("packages.txt");
    if (!packagesFile.is_open()) {
        std::cerr << COLOR_DARK_BLUE << "Error: 'packages.txt' not found in the current directory." << COLOR_RESET << std::endl;
        return;
    }

    std::vector<std::string> packageNames;
    std::string packageName;
    while (std::getline(packagesFile, packageName)) {
        if (!packageName.empty()) {
            packageNames.push_back(packageName);
        }
    }
    packagesFile.close();

    if (packageNames.empty()) {
        std::cout << COLOR_DARK_BLUE << "No packages found in 'packages.txt'. Returning to menu." << COLOR_RESET << std::endl;
        return;
    }

    downloadAndStorePackage(packageNames, dateFolder);
}

int main() {
    int choice;
    std::string dateFolder = getCurrentDate(); // Get the current date in UK format

    // ASCII art in red
    std::cout << COLOR_RED << R"(
░█████╗░██╗░░░░░░█████╗░██╗░░░██╗██████╗░███████╗███╗░░░███╗░█████╗░██████╗░░██████╗
██╔══██╗██║░░░░░██╔══██╗██║░░░██║██╔══██╗██╔════╝████╗░████║██╔══██╗██╔══██╗██╔════╝
██║  ░░╚═╝██║░░░░░███████║██║░░░██║██║░░██║█████╗░░██╔████╔██║██║░░██║██║░░██║╚█████╗░
██║░░██╗██║░░░░░██╔══██║██║░░░██║██║░░██║██╔══╝░░██║╚██╔╝██║██║░░██║██║░░██║░╚═══██╗
╚█████╔╝████ █ ██╗██║░░██║╚██████╔╝██████╔╝███████╗██║░╚═╝░██║╚█████╔╝██████╔╝██████╔╝
░╚════╝░╚══════╝╚═╝░░╚═╝░╚═════╝░╚═════╝░╚══════╝╚═╝░░░░░╚═╝░╚════╝░╚════╝░╚═════╝░
)" << COLOR_RESET << std::endl;

// Message under ASCII art in dark blue
std::cout << COLOR_DARK_BLUE << "claudemods Arch Utility v1.0 21-02-2025" << COLOR_RESET << std::endl;

while (true) {
    // Display the main menu in dark blue
    std::cout << COLOR_DARK_BLUE << "\nMenu:" << COLOR_RESET << "\n";
    std::cout << COLOR_DARK_BLUE << "1. Download and store .pkg.tar.zst packages." << COLOR_RESET << "\n";
    std::cout << COLOR_DARK_BLUE << "2. Download and install packages." << COLOR_RESET << "\n";
    std::cout << COLOR_DARK_BLUE << "3. Download packages from list in packages.txt." << COLOR_RESET << "\n";
    std::cout << COLOR_DARK_BLUE << "4. Only store package names in the log." << COLOR_RESET << "\n";
    std::cout << COLOR_DARK_BLUE << "5. Install packages from the log." << COLOR_RESET << "\n";
    std::cout << COLOR_DARK_BLUE << "6. Download and store a package, then create SquashFS." << COLOR_RESET << "\n";
    std::cout << COLOR_DARK_BLUE << "7. Download and store all installed packages, then create SquashFS." << COLOR_RESET << "\n";
    std::cout << COLOR_DARK_BLUE << "8. Open logs." << COLOR_RESET << "\n";
    std::cout << COLOR_DARK_BLUE << "Enter your choice: " << COLOR_RESET;
    std::cin >> choice;
    std::cin.ignore();

    if (choice == 1) {
        std::string input;
        std::cout << COLOR_DARK_BLUE << "Enter the package names (separated by spaces): " << COLOR_RESET;
        std::getline(std::cin, input);

        std::vector<std::string> packageNames = splitInput(input);
        if (packageNames.empty()) {
            std::cout << COLOR_DARK_BLUE << "No package names provided. Returning to menu." << COLOR_RESET << "\n";
            continue;
        }

        downloadAndStorePackage(packageNames, dateFolder);
    } else if (choice == 2) {
        std::string input;
        std::cout << COLOR_DARK_BLUE << "Enter the package names (separated by spaces): " << COLOR_RESET;
        std::getline(std::cin, input);

        std::vector<std::string> packageNames = splitInput(input);
        if (packageNames.empty()) {
            std::cout << COLOR_DARK_BLUE << "No package names provided. Returning to menu." << COLOR_RESET << "\n";
            continue;
        }

        // Implement download and install logic here
        std::cout << COLOR_DARK_BLUE << "Downloading and installing packages..." << COLOR_RESET << "\n";
    } else if (choice == 3) {
        downloadPackagesFromList(dateFolder);
    } else if (choice == 4) {
        std::string input;
        std::cout << COLOR_DARK_BLUE << "Enter the package names (separated by spaces): " << COLOR_RESET;
        std::getline(std::cin, input);

        std::vector<std::string> packageNames = splitInput(input);
        if (packageNames.empty()) {
            std::cout << COLOR_DARK_BLUE << "No package names provided. Returning to menu." << COLOR_RESET << "\n";
            continue;
        }

        storePackageNamesInLog(packageNames);
    } else if (choice == 6) {
        std::string packageName;
        std::cout << COLOR_DARK_BLUE << "Enter the package name to download and create SquashFS: " << COLOR_RESET;
        std::getline(std::cin, packageName);

        if (packageName.empty()) {
            std::cout << COLOR_DARK_BLUE << "No package name provided. Returning to menu." << COLOR_RESET << "\n";
            continue;
        }

        // Download and store the package
        downloadAndStorePackage({packageName}, dateFolder);

        // Create SquashFS for the package
        createSquashFSForPackage(packageName, dateFolder);

        // Create SquashFS for the entire packages folder
        createSquashFSForPackagesFolder();
    } else if (choice == 7) {
        // Download and store all installed packages
        std::cout << COLOR_DARK_BLUE << "Fetching list of installed packages..." << COLOR_RESET << std::endl;

        // Get the list of installed packages
        std::vector<std::string> installedPackages;
        std::string command = "pacman -Q | awk '{print $1}'";
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            std::cerr << COLOR_DARK_BLUE << "Error fetching installed packages." << COLOR_RESET << std::endl;
            continue;
        }

        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string packageName(buffer);
            packageName.erase(packageName.find_last_not_of(" \n\r\t") + 1);
            installedPackages.push_back(packageName);
        }
        pclose(pipe);

        if (installedPackages.empty()) {
            std::cerr << COLOR_DARK_BLUE << "No installed packages found." << COLOR_RESET << std::endl;
            continue;
        }

        downloadAndStorePackage(installedPackages, dateFolder);

        // Create SquashFS for the entire packages folder
        createSquashFSForPackagesFolder();
    } else if (choice == 8) {
        // Open logs
        std::cout << COLOR_DARK_BLUE << "Opening logs..." << COLOR_RESET << "\n";

        // List available log files
        std::vector<std::string> logFiles;
        int index = 1;

        for (const auto &entry : fs::directory_iterator(".")) {
            if (entry.path().extension() == ".txt") {
                std::cout << COLOR_DARK_BLUE << index << ". " << entry.path().filename().string() << COLOR_RESET << "\n";
                logFiles.push_back(entry.path().filename().string());
                ++index;
            }
        }

        if (logFiles.empty()) {
            std::cout << COLOR_DARK_BLUE << "No logs found." << COLOR_RESET << "\n";
            continue;
        }

        // Prompt the user to select a log file
        int logChoice;
        std::cout << COLOR_DARK_BLUE << "Enter the number of the log to open: " << COLOR_RESET;
        std::cin >> logChoice;
        std::cin.ignore();

        if (logChoice < 1 || static_cast<size_t>(logChoice) > logFiles.size()) {
            std::cout << COLOR_DARK_BLUE << "Invalid choice." << COLOR_RESET << "\n";
            continue;
        }

        std::string logFileName = logFiles[logChoice - 1];
        std::ifstream logFile(logFileName);
        if (!logFile.is_open()) {
            std::cerr << COLOR_DARK_BLUE << "Error opening log file: " << logFileName << COLOR_RESET << std::endl;
            continue;
        }

        std::string line;
        std::cout << COLOR_GREEN << "Contents of " << logFileName << ":" << COLOR_RESET << "\n";
        while (std::getline(logFile, line)) {
            std::cout << line << "\n";
        }

        logFile.close();
    } else {
        std::cout << COLOR_DARK_BLUE << "Invalid choice. Please try again." << COLOR_RESET << "\n";
    }
}

return 0;
}
