#include <exception>
#include <ios>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <ostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <algorithm>

// include compresionbobilotek
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

namespace fs = std::filesystem;


void compressFiles(const std::string& sourceDir, const std::string& targetDir) {
    for (const auto& entry : fs::recursive_directory_iterator(sourceDir)) {
        const auto& sourcePath = entry.path();
        auto relativPath = sourcePath.lexically_relative(fs::path(sourceDir));
        auto targetPath = fs::path(targetDir) / (relativPath.string() + ".gz");

        if (fs::is_regular_file(sourcePath)) {
            fs::create_directories(targetPath.parent_path());

            // Dateistrom zum Lesen öffnen
            std::ifstream sourceFile(sourcePath, std::ios_base::binary);
            if (!sourceFile.is_open()) {
                std::cerr << "Failed to open source file: " << sourcePath << std::endl;
                continue; // Überspringen Sie diese Datei und fahren Sie mit der nächsten fort
            }

            // Dateistrom zum Schreiben als .gz-Datei öffnen
            std::ofstream targetFile(targetPath.string(), std::ios_base::binary);
            if (!targetFile.is_open()) {
                std::cerr << "Failed to open target file: " << targetPath << std::endl;
                continue; // Überspringen Sie diese Datei und fahren Sie mit der nächsten fort
            }

            // Filtering Stream einrichten
            boost::iostreams::filtering_ostream out;
            out.push(boost::iostreams::gzip_compressor());
            out.push(targetFile);

            // Datei kopieren und komprimieren
            try {
                boost::iostreams::copy(sourceFile, out);
                std::cout << "File compressed: " << sourcePath << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error compressing file: " << e.what() << std::endl;
            }

            // Dateistrom schließen
            sourceFile.close();
            targetFile.close();
        }
    }
}


void scanDirectory(const std::string& directory) {
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_directory()) {
            fs::create_directories(entry.path());
        }
    }
}
void copyFiles(const std::string& sourceDir, const std::string& targetDir) {
    for (const auto& entry : fs::recursive_directory_iterator(sourceDir)) {
        const auto& sourcePath = entry.path();
        auto relativPath = sourcePath.lexically_relative(fs::path(sourceDir));
        auto targetPath = fs::path(targetDir) / relativPath;

        if (fs::is_regular_file(sourcePath)) {
            fs::create_directories(targetPath.parent_path());
            fs::copy_file(sourcePath, targetPath, fs::copy_options::overwrite_existing);
        }
    }
}
void listFiles(const std::string& directory) {
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.path())) {
            std::cout << entry.path() << std::endl;

        }
    }
}


int main() {
    std::string sourceDir;
    std::cout << "Enter the source directory:";
    std::cin >> sourceDir;


    const std::string targetDir = "/run/media/lukase/USB-STICK/Test1";

    try {
    compressFiles(sourceDir, targetDir);
    std::cout << "Backup completed successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error " << e.what() << std::endl;
        return 1;
    
    }
    
    return 0;
}