/**
 * @file StringUtils.hpp
 * @brief String utility functions
 */

#pragma once

#include <string>

namespace router {
namespace utils {

/** Utility class for string operations */
class StringUtils {
public:
    /** Replace all occurrences of a substring */
    static std::string replaceAll(std::string str, const std::string& from, const std::string& to);

    /** Sanitize filename by removing dangerous characters */
    static std::string sanitizeFilename(std::string filename);

    /** Replace a placeholder in HTML template */
    static std::string replacePlaceholder(std::string html, const std::string& placeholder, const std::string& replacement);

private:
    StringUtils() = delete; // Static class
};

} // namespace utils
} // namespace router
