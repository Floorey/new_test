#include <exception>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

// Include for Boost Iostreams for compression
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

namespace fs = std::filesystem;

// Mutex for critical sections
std::mutex mtx;

// Function to compress a single file
void compressFiles(const std::string& sourceDir, const std::string& targetDir, const std::vector<std::string>& includeExtensions, const std::vector<std::string>& excludeExtensions) {
    for (const auto& entry : fs::recursive_directory_iterator(sourceDir)) {
        const auto& sourcePath = entry.path();
        auto relativePath = sourcePath.lexically_relative(fs::path(sourceDir));
        auto targetPath = fs::path(targetDir) / (relativePath.string() + ".gz");

        if (fs::is_regular_file(sourcePath)) {
            // Check if the file extension should be included or excluded
            std::string extension = sourcePath.extension().string();
            bool include = includeExtensions.empty(); // If includeExtensions is empty, include all files by default
            for (const auto& includeExt : includeExtensions) {
                if (extension == includeExt) {
                    include = true;
                    break;
                }
            }
            for (const auto& excludeExt : excludeExtensions) {
                if (extension == excludeExt) {
                    include = false;
                    break;
                }
            }

            if (!include) {
                std::cout << "Skipping file: " << sourcePath << std::endl;
                continue;
            }

            fs::create_directories(targetPath.parent_path());

            // Open file stream for reading
            std::ifstream sourceFile(sourcePath, std::ios_base::binary);
            if (!sourceFile.is_open()) {
                std::cerr << "Failed to open source file: " << sourcePath << std::endl;
                continue; // Skip this file and move to the next one
            }

            // Open file stream for writing as .gz file
            std::ofstream targetFile(targetPath.string(), std::ios_base::binary);
            if (!targetFile.is_open()) {
                std::cerr << "Failed to open target file: " << targetPath << std::endl;
                continue; // Skip this file and move to the next one
            }

            // Setup filtering stream
            boost::iostreams::filtering_ostream out;
            out.push(boost::iostreams::gzip_compressor());
            out.push(targetFile);

            // Copy file and compress
            try {
                boost::iostreams::copy(sourceFile, out);
                std::cout << "File compressed: " << sourcePath << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error compressing file: " << e.what() << std::endl;
            }

            // Close file streams
            sourceFile.close();
            targetFile.close();
        }
    }
}

// Function executed by each thread to process files
void processFiles(const std::vector<fs::path>& files, const std::string& sourceDir, const std::string& targetDir) {
    for (const auto& sourcePath : files) {
        auto relativePath = sourcePath.lexically_relative(fs::path(sourceDir));
        auto targetPath = fs::path(targetDir) / (relativePath.string() + ".gz");

        if (fs::is_regular_file(sourcePath)) {
            fs::create_directories(targetPath.parent_path());

            std::ifstream sourceFile(sourcePath, std::ios_base::binary);
            if (!sourceFile.is_open()){
                std::cerr << "Failed to open source file:" << sourcePath << std::endl;
                continue;
            }

            std::ofstream targetFile(targetPath.string(), std::ios_base::binary);
            if (!targetFile.is_open()) {
                std::cerr << "Failed to open target file: " << targetPath << std::endl;
                continue;
            }

            boost::iostreams::filtering_ostream out;
            out.push(boost::iostreams::gzip_compressor());
            out.push(targetFile);

            try {
                boost::iostreams::copy(sourceFile, out);
                std::cout << "File compressed: " << sourcePath << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error compressing file: " << e.what() << std::endl;
            }

            sourceFile.close();
            targetFile.close();
        }
    }
}

void listFiles(const std::string& directory, std::vector<fs::path>& files, const std::vector<std::string>& excludedExtensions) {
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.path())) {
            bool excludeFile = false;
            for (const auto& extension : excludedExtensions)  {
                if (entry.path().extension() == "." + extension) {
                    excludeFile = true;
                    break;
                }
            }
            if(!excludeFile) {
                files.push_back(entry.path());
            }
        }
    }
}

int main() {
    std::string sourceDir;
    std::cout << "Enter the source directory:";
    std::cin >> sourceDir;

    std::cout << "Files in source directory: " << std::endl; 
    std::vector<fs::path> files;
    listFiles(sourceDir, files, {});

    const std::string targetDir = "/run/media/lukase/USB-STICK/Test1";

    // Number of threads equal to the number of CPU cores
    unsigned int numThreads = std::thread::hardware_concurrency();

    // Group files into partial lists for each thread
    std::vector<std::vector<fs::path>> fileChunks(numThreads);
    for (size_t i = 0; i < files.size(); ++i) {
        fileChunks[i % numThreads].push_back(files[i]);
    }

    // Start threads for each partial list of files
    std::vector<std::thread> threads;
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back(processFiles, fileChunks[i], sourceDir, targetDir);
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Backup completed successfully." << std::endl;

    return 0;
}
