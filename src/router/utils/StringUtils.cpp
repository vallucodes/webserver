/**
 * @file StringUtils.cpp
 * @brief String utility functions implementation
 */

#include "StringUtils.hpp"
#include <algorithm>

namespace router {
namespace utils {

std::string StringUtils::replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

std::string StringUtils::sanitizeFilename(std::string filename) {
    const std::string forbiddenChars = "/\\:*?\"<>|";
    filename.erase(std::remove_if(filename.begin(), filename.end(),
        [&forbiddenChars](char c) {
            return forbiddenChars.find(c) != std::string::npos;
        }), filename.end());
    return filename;
}

std::string StringUtils::replacePlaceholder(std::string html, const std::string& placeholder, const std::string& replacement) {
    size_t pos = 0;
    while ((pos = html.find(placeholder, pos)) != std::string::npos) {
        html.replace(pos, placeholder.length(), replacement);
        pos += replacement.length();
    }
    return html;
}

} // namespace utils
} // namespace router
