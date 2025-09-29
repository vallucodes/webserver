#include "HandlerUtils.hpp"
#include "../utils/HttpResponseBuilder.hpp"
#include "../utils/StringUtils.hpp"
#include "../utils/Utils.hpp"
#include "../HttpConstants.hpp"
#include <filesystem>
#include <fstream>

namespace router::handlers {

// Location finding utilities
const Location* HandlerUtils::findUploadLocation(const std::string& requestPath, const Server& server) {
  for (const auto& loc : server.getLocations()) {
    if (std::string(requestPath).find(loc.location) == 0 && !loc.upload_path.empty()) {
      return &loc;
    }
  }
  return nullptr;
}

const Location* HandlerUtils::findCgiLocation(const std::string& requestPath, const Server& server) {
  for (const auto& loc : server.getLocations()) {
    if (std::string(requestPath).find(loc.location) == 0 && !loc.cgi_path.empty() && !loc.cgi_ext.empty()) {
      return &loc;
    }
  }
  return nullptr;
}

const Location* HandlerUtils::findRedirectLocation(const std::string& requestPath, const Server& server) {
  for (const auto& loc : server.getLocations()) {
    if (std::string(requestPath).find(loc.location) == 0 && !loc.return_url.empty()) {
      return &loc;
    }
  }
  return nullptr;
}

const Location* HandlerUtils::findBestMatchingLocation(const std::string& requestPath, const Server& server) {
  const Location* bestLocation = nullptr;
  size_t bestMatchLength = 0;

  for (const auto& loc : server.getLocations()) {
    if (requestPath.find(loc.location) == 0 && loc.location.length() > bestMatchLength) {
      bestLocation = &loc;
      bestMatchLength = loc.location.length();
    }
  }

  return bestLocation;
}

// Request processing utilities
std::string HandlerUtils::processRequestBody(const Request& req) {
  std::string processedBody(req.getBody());
  if (router::utils::isChunked(req)) {
    processedBody = router::utils::parseChunkedRequestBody(processedBody);
  }
  return processedBody;
}

bool HandlerUtils::validateContentType(const std::vector<std::string>& contentTypeKey, const std::string& expectedType) {
  if (contentTypeKey.empty()) {
    return false;
  }
  return contentTypeKey[0].find(expectedType) != std::string::npos;
}

std::string HandlerUtils::extractBoundary(const std::string& contentType) {
  const size_t boundaryPos = contentType.find("boundary=");
  if (boundaryPos == std::string::npos) {
    return "";
  }
  return "--" + contentType.substr(boundaryPos + 9);
}

// File operations
bool HandlerUtils::writeFileToDisk(const std::string& filePath, const std::string& content) {
  std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());

  std::ofstream outFile(filePath, std::ios::binary);
  if (!outFile) {
    return false;
  }

  outFile.write(content.c_str(), content.length());
  outFile.close();
  return true;
}

bool HandlerUtils::deleteFileFromDisk(const std::string& filePath) {
  return std::filesystem::remove(filePath);
}

std::string HandlerUtils::resolveFilePath(const std::string& filename, const Location* location, const std::string& serverRoot) {
  if (!location) {
    return "";
  }

  std::string uploadPath = router::utils::StringUtils::resolvePath(location->upload_path, serverRoot);
  return uploadPath + "/" + filename;
}

// Validation utilities
bool HandlerUtils::isValidFilename(const std::string& filename) {
  return !filename.empty() && filename.find("..") == std::string::npos && filename.find("/") == std::string::npos;
}

bool HandlerUtils::isValidFileSize(size_t fileSize, size_t maxSize) {
  return fileSize <= maxSize;
}

// Response utilities
void HandlerUtils::setConnectionHeaders(Response& res, const Request& req) {
  if (router::utils::shouldKeepAlive(req)) {
    res.setHeaders(http::CONNECTION, http::CONNECTION_KEEP_ALIVE);
  } else {
    res.setHeaders(http::CONNECTION, http::CONNECTION_CLOSE);
  }
}

void HandlerUtils::setErrorResponse(Response& res, int statusCode, const Request& req, const Server& server) {
  router::utils::HttpResponseBuilder::setErrorResponse(res, statusCode, req, server);
}

void HandlerUtils::setSuccessResponse(Response& res, int statusCode, const Request& req) {
  router::utils::HttpResponseBuilder::setSuccessResponseWithDefaultPage(res, statusCode, req);
}

} // namespace router::handlers
