/**
 * @file FileUtils.hpp
 * @brief File utility functions for the router
 */

#pragma once

#include <string>

namespace router {
namespace utils {

/**
 * @class FileUtils
 * @brief Utility class for file operations
 */
class FileUtils {
public:
    /**
     * @brief Read entire file content into a string
     * @param filename Path to the file to read
     * @return String containing the complete file contents
     * @throws std::runtime_error if file cannot be opened or read
     */
    static std::string readFileToString(const std::string& filename);

    /**
     * @brief Get MIME content type for a file based on its extension
     * @param filePath Path to the file
     * @return MIME type string (e.g., "text/html", "image/jpeg")
     */
    static std::string getContentType(const std::string& filePath);

private:
    FileUtils() = delete; // Static class
};

} // namespace utils
} // namespace router
