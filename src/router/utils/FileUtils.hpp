/**
 * @file FileUtils.hpp
 * @brief File utility functions
 */

#pragma once

#include <string>

namespace router {
namespace utils {

/** Utility class for file operations */
class FileUtils {
public:
    /** Read entire file content into a string */
    static std::string readFileToString(const std::string& filename);

    /** Get MIME content type for a file based on its extension */
    static std::string getContentType(const std::string& filePath);

private:
    FileUtils() = delete; // Static class
};

} // namespace utils
} // namespace router
