/**
 * @file StringUtils.hpp
 * @brief String utility functions for the router
 */

#pragma once

#include <string>

namespace router {
namespace utils {

/**
 * @class StringUtils
 * @brief Utility class for string operations
 */
class StringUtils {
public:
    /**
     * @brief Replace all occurrences of a substring in a string
     * @param str The string to modify
     * @param from The substring to replace
     * @param to The replacement string
     * @return Modified string with all occurrences replaced
     */
    static std::string replaceAll(std::string str, const std::string& from, const std::string& to);

    /**
     * @brief Sanitize filename by removing dangerous characters
     * @param filename The filename to sanitize
     * @return Sanitized filename safe for filesystem operations
     */
    static std::string sanitizeFilename(std::string filename);

    /**
     * @brief Replace a placeholder in HTML template
     * @param html The HTML string to modify
     * @param placeholder The placeholder to replace
     * @param replacement The replacement text
     * @return Modified HTML string
     */
    static std::string replacePlaceholder(std::string html, const std::string& placeholder, const std::string& replacement);

private:
    StringUtils() = delete; // Static class
};

} // namespace utils
} // namespace router
