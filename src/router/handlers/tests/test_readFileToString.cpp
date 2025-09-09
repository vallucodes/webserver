#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <fstream>
#include <sstream>

// Helper functions from Handlers.cpp
std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string getContentType(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string ext = filename.substr(dotPos + 1);

        // Convert to lowercase for case-insensitive comparison
        for (char& c : ext) {
            c = std::tolower(c);
        }

        // Use switch-like behavior with computed hash
        switch (ext.length()) {
            case 3:
                if (ext == "css") return "text/css";
                if (ext == "png") return "image/png";
                if (ext == "jpg") return "image/jpeg";
                if (ext == "gif") return "image/gif";
                if (ext == "htm") return "text/html";
                if (ext == "txt") return "text/plain";
                break;
            case 4:
                if (ext == "html") return "text/html";
                if (ext == "jpeg") return "image/jpeg";
                if (ext == "json") return "application/json";
                break;
            case 2:
                if (ext == "js") return "application/javascript";
                break;
        }
    }
    return "text/plain";
}

int main() {
    std::cout << "Testing readFileToString and getContentType functions..." << std::endl;

    // Test readFileToString with existing file
    try {
        std::string content = readFileToString("example.txt");
        std::cout << "File contents:\n" << content << std::endl;
        std::cout << "readFileToString: File read successfully!" << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "readFileToString Error: " << e.what() << std::endl;
    }

    // Test readFileToString with non-existent file
    try {
        std::string content = readFileToString("nonexistent.txt");
        std::cout << "File contents:\n" << content << std::endl;
    } catch (const std::runtime_error& e) {
        std::cout << "readFileToString: Expected error for non-existent file: " << e.what() << std::endl;
    }

    // Test getContentType function
    std::cout << "\nTesting getContentType function:" << std::endl;

    std::vector<std::string> testFiles = {
        "index.html", "style.css", "script.js", "data.json",
        "image.png", "photo.jpg", "picture.jpeg", "anim.gif",
        "page.htm", "document.txt", "unknown.xyz"
    };

    for (const auto& file : testFiles) {
        std::string contentType = getContentType(file);
        std::cout << file << " -> " << contentType << std::endl;
    }

    std::cout << "\nAll tests done!" << std::endl;
    return 0;
}
