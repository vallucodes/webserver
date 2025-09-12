#include <string>
#include <fstream>
#include <sstream>

// Utility function for reading files
std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
