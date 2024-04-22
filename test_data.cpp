#include <exception>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <stdexcept>

namespace fs = std::filesystem;

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
int main() {
    const std::string sourceDir = "/home/lukase/Dokumente";
    const std::string targetDir = "/run/media/lukase/USB-STICK/Test1";

    try {
    scanDirectory(sourceDir);
    copyFiles(sourceDir, targetDir);
    std::cout << "Backup completed successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}