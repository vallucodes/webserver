/**
 * @file StringUtils.hpp
 * @brief String utility functions
 */

#pragma once

#include <string> // for std::string
#include <string_view> // for std::string_view

// Forward declaration
struct Location;

namespace router {
namespace utils {

/** Utility class for string operations */
class StringUtils {
public:
    /** Replace all occurrences of a substring */
    static std::string replaceAll(std::string str, const std::string& from, const std::string& to);

    /** Sanitize filename by removing dangerous characters */
    // static std::string sanitizeFilename(std::string filename);

    /** Replace a placeholder in HTML template */
    static std::string replacePlaceholder(std::string html, const std::string& placeholder, const std::string& replacement);

    /** Normalize path by collapsing multiple consecutive slashes */
    static std::string normalizePath(std::string path);

    /** Determine file path based on request path and location */
    static std::string determineFilePath(const std::string_view& path, const Location* location, const std::string& server_root);

    /** Resolve path relative to server root (nginx-style) */
    static std::string resolvePath(const std::string& path, const std::string& server_root);

private:
    StringUtils() = delete; // Static class
};

} // namespace utils
} // namespace router
