#pragma once

#include "../../server/Server.hpp"
#include "../../request/Request.hpp"
#include "../../response/Response.hpp"
#include "../HttpConstants.hpp"

namespace router::handlers {

/**
 * @brief Common utilities for HTTP handlers
 */
class HandlerUtils {
public:
  // Location finding utilities
  static const Location* findUploadLocation(const std::string& requestPath, const Server& server);
  static const Location* findCgiLocation(const std::string& requestPath, const Server& server);
  static const Location* findRedirectLocation(const std::string& requestPath, const Server& server);
  static const Location* findBestMatchingLocation(const std::string& requestPath, const Server& server);

  // Request processing utilities
  static std::string processRequestBody(const Request& req);
  static bool validateContentType(const std::vector<std::string>& contentTypeKey, const std::string& expectedType);
  static std::string extractBoundary(const std::string& contentType);

  // File operations
  static bool writeFileToDisk(const std::string& filePath, const std::string& content);
  static bool deleteFileFromDisk(const std::string& filePath);
  static std::string resolveFilePath(const std::string& filename, const Location* location, const std::string& serverRoot);

  // Validation utilities
  static bool isValidFilename(const std::string& filename);
  static bool isValidFileSize(size_t fileSize, size_t maxSize = 1024 * 1024);

  // Response utilities
  static void setConnectionHeaders(Response& res, const Request& req);
  static void setErrorResponse(Response& res, int statusCode, const Request& req, const Server& server);
  static void setSuccessResponse(Response& res, int statusCode, const Request& req);
};

} // namespace router::handlers
