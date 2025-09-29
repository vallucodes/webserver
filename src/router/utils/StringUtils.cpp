/**
 * @file StringUtils.cpp
 * @brief String utility functions implementation
 */

#include "StringUtils.hpp"
#include "../HttpConstants.hpp"
#include "../../server/Server.hpp" // for Location struct definition
#include <algorithm> // for std::remove_if, std::find_if

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

std::string StringUtils::determineFilePathCGI(const std::string_view& path, const Location* location, const std::string& server_root) {
  if (path == page::ROOT_HTML || path == page::INDEX_HTML_PATH) {
    return page::INDEX_HTML;
  }
  else {
    // Use location-configured CGI path
    // Strip the location prefix from the request path
    std::string requestPath = std::string(path);
    std::string locationPrefix = location->location;

    // Remove the location prefix from the request path
    if (requestPath.length() > locationPrefix.length() &&
        requestPath.substr(0, locationPrefix.length()) == locationPrefix) {
      requestPath = requestPath.substr(locationPrefix.length());

      // Remove leading slash if present after stripping prefix
      if (!requestPath.empty() && requestPath[0] == '/') {
        requestPath = requestPath.substr(1);
      }
    }

    // Resolve CGI path (nginx-style)
    std::string cgiPath = StringUtils::resolvePath(location->cgi_path, server_root);

    // Ensure cgiPath ends with slash for proper concatenation
    if (!cgiPath.empty() && cgiPath.back() != '/') {
      cgiPath += '/';
    }

    return cgiPath + requestPath;
  }
}

std::string StringUtils::determineFilePathBasic(const std::string& requestPath) {
  if (requestPath == page::ROOT_HTML || requestPath == page::INDEX_HTML_PATH) {
    return page::INDEX_HTML;
  }

  return page::WWW + requestPath;
}

std::string StringUtils::replacePlaceholder(std::string html, const std::string& placeholder, const std::string& replacement) {
  size_t pos = 0;
  while ((pos = html.find(placeholder, pos)) != std::string::npos) {
    html.replace(pos, placeholder.length(), replacement);
    pos += replacement.length();
  }
  return html;
}

// read: https://nginx.org/en/docs/http/ngx_http_core_module.html?utm_source=chatgpt.com#merge_slashes
std::string StringUtils::normalizePath(std::string path) {
  // collapse multiple slashes
  size_t i = 0, j = 0;
  while (i < path.size()) {
    path[j++] = path[i++];
    if (path[j-1] == '/') {
      while (i < path.size() && path[i] == '/') i++; // skip duplicates
    }
  }
  path.resize(j);
  if (path.empty()) path = "/";
  return path;
}
/**
 * @brief Resolve path relative to server root (nginx-style)
 * @param path Path from configuration (can be relative or absolute)
 * @param server_root Server root directory
 * @return Resolved absolute path
 */
std::string StringUtils::resolvePath(const std::string& path, const std::string& server_root) {
  if (path.empty()) {
    return server_root;
  }

  // If path starts with server_root, it's a full absolute path - use as-is
  if (path.find(server_root) == 0) {
    return path;
  }

  // If path starts with '/', it's relative to server root - append to server root
  if (path[0] == '/') {
    std::string resolved = server_root;
    if (!resolved.empty() && resolved.back() != '/') {
      resolved += '/';
    }
    resolved += path.substr(1); // Remove leading '/'
    return resolved;
  }

  // Relative path - resolve against server root
  std::string resolved = server_root;
  if (!resolved.empty() && resolved.back() != '/') {
    resolved += '/';
  }
  resolved += path;

  return resolved;
}

} // namespace utils
} // namespace router
