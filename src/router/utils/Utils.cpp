#include "Utils.hpp"
#include "../../server/Server.hpp"
#include "../HttpConstants.hpp"
#include "HttpResponseBuilder.hpp"
#include "StringUtils.hpp"
#include "FileUtils.hpp"

#include <algorithm> // for std::transform
#include <cctype> // for std::tolower
#include <filesystem> // for std::filesystem
#include <iostream> // for std::cout
#include <chrono> // for std::chrono::system_clock::to_time_t, std::chrono::file_clock::to_sys
#include <ctime> // for std::strftime, std::localtime

namespace router {
namespace utils {


/** Determine if connection should be kept alive based on HTTP version and request headers */
bool shouldKeepAlive(const Request& req) {
  // Check if client explicitly requests connection close
  auto connectionHeaders = req.getHeaders("connection");
  if (!connectionHeaders.empty()) {
    std::string connectionValue = connectionHeaders[0];
    // Convert to lowercase for case-insensitive comparison
    std::transform(connectionValue.begin(), connectionValue.end(), connectionValue.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if (connectionValue == "close") {
      return false;
    }
    if (connectionValue == "keep-alive") {
      return true;
    }
  }

  // HTTP/1.1 defaults to keep-alive, HTTP/1.0 defaults to close
  std::string httpVersion = std::string(req.getHttpVersion());
  if (httpVersion == "HTTP/1.1") {
    return true;
  }

  return false;
}

/** Check if the request is chunked */
bool isChunked(const Request& req) {
  // READ: https://www.rfc-editor.org/rfc/rfc7230#section-3.3.1
  auto transferEncoding = req.getHeaders("transfer-encoding");
  // example: transfer-encoding: chunked
  bool findChunk = false;
  if (!transferEncoding.empty()) {
    // example: transfer-encoding: chunked -> true
    for (const auto& encoding : transferEncoding) {
      std::string lowerEncoding = encoding;
      std::transform(lowerEncoding.begin(), lowerEncoding.end(), lowerEncoding.begin(), ::tolower);
      if (lowerEncoding.find("chunked") != std::string::npos) {
        findChunk = true;
        break;
      }
    }
  }
  return findChunk;
}

bool isCgiScriptWithLocation(const std::string& filename, const Location* location) {
  if (!location || location->cgi_ext.empty()) {
    return false;
  }

  size_t dotPos = filename.find_last_of('.');
  if (dotPos != std::string::npos) {
    // Get the extension of the filename
    std::string ext = filename.substr(dotPos + 1);
    // Convert to lowercase for case-insensitive comparison
    for (char& c : ext) {
      c = std::tolower(c);
    }

    // Check against location-configured extensions
    for (const auto& allowedExt : location->cgi_ext) {
      std::string allowedExtLower = allowedExt;
      // Convert to lowercase for case-insensitive comparison
      for (char& c : allowedExtLower) {
        c = std::tolower(c);
      }
      // Remove leading dot from allowed extension for comparison
      if (allowedExtLower.length() > 0 && allowedExtLower[0] == '.') {
        allowedExtLower = allowedExtLower.substr(1);
      }
      if (ext == allowedExtLower) {
        return true;
      }
    }
  }
  return false;
}

/** Parse chunked request body */
std::string parseChunkedRequestBody(const std::string& body) {
  std::string result;
  size_t pos = 0;

  while (pos < body.length()) {
    // Find the end of the chunk size line
    // \r is carriage return, \n is line feed (CRLF)
    size_t lineEnd = body.find("\r\n", pos);
    if (lineEnd == std::string::npos) {
      lineEnd = body.find("\n", pos);
      if (lineEnd == std::string::npos) break;
    }

    // Extract chunk size
    std::string chunkSizeStr = body.substr(pos, lineEnd - pos);
    size_t chunkSize = 0;

    /*
    1f\r\n
    abcdefghijklmnopqrstuvwxyz123\r\n
    0\r\n
    \r\n
    */
    // example: 1f\r\n -> 31
    // read 31 bytes → "abcdefghijklmnopqrstuvwxyz123"
    // example: 0\r\n -> 0
    // read 0 bytes → ""
    try {
      // should be a valid hexadecimal number
      // convrt to unsigned long with base 16
      chunkSize = std::stoul(chunkSizeStr, nullptr, 16);
    } catch (...) {
      break; // Invalid chunk size
    }

    // If chunk size is 0, no more chunks
    if (chunkSize == 0) break;

    // Move past the chunk size line
    // start from the next line
    pos = lineEnd + (body[lineEnd] == '\r' ? 2 : 1);

    // Extract chunk data
    if (pos + chunkSize <= body.length()) {
      result += body.substr(pos, chunkSize);
      pos += chunkSize;

      // Skip the trailing CRLF
      if (pos + 2 <= body.length() && body.substr(pos, 2) == "\r\n") {
        pos += 2;
      } else if (pos + 1 <= body.length() && body[pos] == '\n') {
        pos += 1;
      }
    } else {
      break; // Invalid chunk
    }
  }
  // example: "abcdefghijklmnopqrstuvwxyz123"
  return result;
}


/** Set up CGI environment variables */
// READ: https://datatracker.ietf.org/doc/html/rfc3875
std::vector<std::string> setupCgiEnvironment(const Request& req, const std::string& scriptPath, const std::string& scriptName, const Server& server) {
  std::vector<std::string> env;

  // Basic CGI environment variables
  env.push_back("GATEWAY_INTERFACE=CGI/1.1");
  env.push_back("SERVER_PROTOCOL=HTTP/1.1");
  env.push_back("REQUEST_METHOD=" + std::string(req.getMethod()));
  env.push_back("SCRIPT_NAME=" + scriptName);
  env.push_back("SCRIPT_FILENAME=" + scriptPath);

  // PATH_INFO and PATH_TRANSLATED
  std::string pathStr(req.getPath());
  // Find the position of the query string
  // example: /cgi-bin/script.py?name=Ilia&age=43
  size_t queryPos = pathStr.find('?');
  // If there is a query string, extract the path before it
  // example: /cgi-bin/script.py
  std::string pathWithoutQuery = (queryPos != std::string::npos) ? pathStr.substr(0, queryPos) : pathStr;

  // PATH_INFO - should be the path portion after the script name
  std::string pathInfo = "";
  // For /cgi-bin/hello.py, scriptName is "/cgi-bin/hello.py", so PATH_INFO should be empty
  // For /cgi-bin/script.py/extra/path, scriptName is "/cgi-bin/script.py", so PATH_INFO should be "/extra/path"
  if (pathWithoutQuery.length() > scriptName.length() &&
      pathWithoutQuery.substr(0, scriptName.length()) == scriptName) {
    pathInfo = pathWithoutQuery.substr(scriptName.length());
  }
  env.push_back("PATH_INFO=" + pathInfo);
  if (!pathInfo.empty()) {
    env.push_back("PATH_TRANSLATED=" + (scriptPath + pathInfo));
  }

  // Query string handling
  if (queryPos != std::string::npos) {
    // example: /cgi-bin/script.py?name=Ilia&age=43 -> name=Ilia&age=43
    env.push_back("QUERY_STRING=" + pathStr.substr(queryPos + 1));
  } else {
    // example: /cgi-bin/script.py -> ""
    env.push_back("QUERY_STRING=");
  }

  // Content handling - handle chunked requests
  std::string body = std::string(req.getBody());
  auto transferEncoding = req.getHeaders("transfer-encoding");
  // example: transfer-encoding: chunked
  bool isChunked = false;

  /*
  HTTP/1.1 200 OK
  Transfer-Encoding: chunked

  4\r\n
  Wiki\r\n
  5\r\n
  pedia\r\n
  0\r\n
  \r\n
  */
  if (!transferEncoding.empty()) {
    // example: transfer-encoding: chunked -> true
    for (const auto& encoding : transferEncoding) {
      // example: transfer-encoding: chunked -> true
      if (encoding.find("chunked") != std::string::npos) {
        isChunked = true;
        break;
      }
    }
  }

  // Unchunk the body if it's chunked
  if (isChunked) {
    body = parseChunkedRequestBody(body);
  }

  auto contentType = req.getHeaders("content-type");
  if (!contentType.empty()) {
    env.push_back("CONTENT_TYPE=" + contentType[0]);
  }

  auto contentLength = req.getHeaders("content-length");
  if (!contentLength.empty()) {
    env.push_back("CONTENT_LENGTH=" + contentLength[0]);
  } else {
    // Use the actual body size
    env.push_back("CONTENT_LENGTH=" + std::to_string(body.length()));
  }

  // Server information - using dynamic values from server config
  env.push_back("SERVER_SOFTWARE=" + network::SERVER_SOFTWARE);
  env.push_back("SERVER_NAME=" + server.getName());
  env.push_back("SERVER_PORT=" + std::to_string(server.getPort()));

  // Remote client info (using constants)
  env.push_back("REMOTE_ADDR=" + network::DEFAULT_REMOTE_ADDR);
  env.push_back("REMOTE_HOST=" + network::DEFAULT_REMOTE_HOST);

  // Add PATH for finding executables
  env.push_back("PATH=" + network::SYSTEM_PATH);

  return env;
}

/** Generate HTML directory listing */
std::string generateDirectoryListing(const std::string& dirPath, const std::string& requestPath, const std::string& serverRoot) {
  // Search for template files in multiple locations
  std::vector<std::string> searchPaths;

  // 1. Current directory
  searchPaths.push_back(StringUtils::normalizePath(dirPath + "/" + page::AUTOINDEX_TEMPLATE));
  searchPaths.push_back(StringUtils::normalizePath(dirPath + "/" + page::AUTOINDEX_FALLBACK));

  // 2. Parent directories (walking up the tree)
  std::string currentPath = dirPath;
  while (currentPath != serverRoot && currentPath.length() > serverRoot.length()) {
    currentPath = currentPath.substr(0, currentPath.find_last_of('/'));
    if (currentPath.length() >= serverRoot.length()) {
      searchPaths.push_back(StringUtils::normalizePath(currentPath + "/" + page::AUTOINDEX_TEMPLATE));
      searchPaths.push_back(StringUtils::normalizePath(currentPath + "/" + page::AUTOINDEX_FALLBACK));
    }
  }

  // 3. Server root as final fallback
  searchPaths.push_back(StringUtils::normalizePath(serverRoot + "/" + page::AUTOINDEX_TEMPLATE));
  searchPaths.push_back(StringUtils::normalizePath(serverRoot + "/" + page::AUTOINDEX_FALLBACK));

  std::string html;
  bool templateFound = false;

  // Try to find and load template
  for (const auto& templatePath : searchPaths) {
    try {
      if (std::filesystem::exists(templatePath)) {
        html = FileUtils::readFileToString(templatePath);
        templateFound = true;
        break;
      }
    } catch (const std::exception& e) {
      // Continue to next path
      continue;
    }
  }

  if (!templateFound) {
    // If no template found anywhere, throw an exception
    std::cout << "Error: Could not find autoindex template in any location" << std::endl;
    throw std::runtime_error("Could not load directory listing template");
  }

  // Replace placeholders
  html = StringUtils::replaceAll(html, "{{PATH}}", requestPath);

  // Generate parent directory link
  std::string parentLink = "";
  if (requestPath != "/") {
    std::string parentPath = requestPath;

    // Remove trailing slash if present
    if (parentPath.back() == '/') {
      parentPath.pop_back();
    }

    // Find the last slash to get the parent directory
    size_t lastSlash = parentPath.find_last_of('/');
    if (lastSlash != std::string::npos) {
      parentPath = parentPath.substr(0, lastSlash);
      if (parentPath.empty()) parentPath = "/";
      parentLink = "  <a href=\"" + parentPath + "\" class=\"back-link\">← Parent directory</a>\n";
    }
  }
  html = StringUtils::replaceAll(html, "{{PARENT_LINK}}", parentLink);

  // Generate directory items
  std::string items = "";
  try {
    for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
      std::string name = entry.path().filename().string();
      std::string linkPath = requestPath;
      if (linkPath.back() != '/') linkPath += '/';
      linkPath += name;

      bool isDir = entry.is_directory();
      std::string icon = isDir ? "📁" : "📄";
      std::string cssClass = isDir ? "dir-icon" : "file-icon";

      // Get file size
      std::string sizeStr = "-";
      if (!isDir) {
        try {
          auto size = entry.file_size();
          if (size < 1024) sizeStr = std::to_string(size) + " B";
          else if (size < 1024 * 1024) sizeStr = std::to_string(size / 1024) + " KB";
          else sizeStr = std::to_string(size / (1024 * 1024)) + " MB";
        } catch (...) {}
      }

      // Get last modified time
      std::string dateStr = "-";
      try {
        auto time = entry.last_write_time();
        auto time_t = std::chrono::system_clock::to_time_t(std::chrono::file_clock::to_sys(time));
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", std::localtime(&time_t));
        dateStr = buffer;
      } catch (...) {}

      items += "    <div class=\"item\">\n";
      items += "      <span class=\"" + cssClass + "\">" + icon + "</span>\n";
      items += "      <a href=\"" + linkPath + "\" class=\"name\">" + name + "</a>\n";
      items += "      <span class=\"size\">" + sizeStr + "</span>\n";
      items += "      <span class=\"date\">" + dateStr + "</span>\n";
      items += "    </div>\n";
    }
  } catch (const std::exception& e) {
    items += "    <div class=\"item\">Error reading directory: " + std::string(e.what()) + "</div>\n";
  }

  html = StringUtils::replaceAll(html, "{{ITEMS}}", items);

  return html;
}

bool handleDirectoryRequest(const std::string& dirPath, const std::string& requestPath,
                            const Location* location, Response& res, const Request& req, const std::string& serverRoot) {
  // Try autoindex first if enabled
  if (location && location->autoindex) {
    try {
      std::string dirListing = generateDirectoryListing(dirPath, requestPath, serverRoot);
      HttpResponseBuilder::setSuccessResponse(res, dirListing, http::CONTENT_TYPE_HTML, req);
      return true;
    } catch (const std::exception& e) {
      // Template loading failed, return 500 error
      std::cout << "Error generating directory listing: " << e.what() << std::endl;
      return false; // This will cause the caller to return 500
    }
  }

  // Combine location-specific, global, and default index files
  std::vector<std::string> indexPaths;
  if (location && !location->index.empty()) {
    std::string indexPath = dirPath;
    if (!indexPath.ends_with('/')) indexPath += '/';
    indexPaths.push_back(indexPath + location->index); // Location-specific index
    indexPaths.push_back(page::WWW + "/" + location->index); // Global index
  }
  for (const auto& defaultFile : page::DEFAULT_INDEX_FILES) {
    indexPaths.push_back(dirPath + "/" + defaultFile); // Default index files
  }

  // Try each index file in order
  for (const auto& path : indexPaths) {
    if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
      std::string fileContent = FileUtils::readFileToString(path);
      std::string contentType = FileUtils::getContentType(path);
      HttpResponseBuilder::setSuccessResponse(res, fileContent, contentType, req);
      return true;
    }
  }

  return false;
}

/**
 * @brief Serve a static file
 * @param filePath Path to the file to serve
 * @param res Response object
 * @param req HTTP request
 * @return true if served successfully
 */
bool serveStaticFile(const std::string& filePath, Response& res, const Request& req) {
  try {
    std::string fileContent = FileUtils::readFileToString(filePath);
    std::string contentType = FileUtils::getContentType(filePath);
    HttpResponseBuilder::setSuccessResponse(res, fileContent, contentType, req);
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

} // namespace utils
} // namespace router
