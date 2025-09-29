/**
 * @file ValidationUtils.cpp
 * @brief Validation utility functions implementation
 */

#include "ValidationUtils.hpp"
#include "HttpResponseBuilder.hpp"
#include "../HttpConstants.hpp"
#include "../../request/Request.hpp"
#include "../../response/Response.hpp"
#include "../../server/Server.hpp"
#include <filesystem> // for std::filesystem::exists, std::filesystem::is_regular_file
#include <algorithm> // for std::find

namespace router {
namespace utils {

bool isValidLocationServer(Response& res, const Location* location, const Server* server, const Request& req) {
  if (!location) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req, *server);
    return false;
  }

  if (!server) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req, *server);
    return false;
  }

  return true;
}

bool isValidPath(const std::string_view& path, Response& res, const Request& req) {
  if (path.empty()) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
    return false;
  }

  // Check for path traversal attempts
  if (path.find("..") != std::string::npos) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::FORBIDDEN_403, req);
    return false;
  }

  return true;
}

bool isValidPath(const std::string_view& path, Response& res, const Request& req, const Server& server) {
  if (path.empty()) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req, server);
    return false;
  }

  // Check for path traversal attempts
  if (path.find("..") != std::string::npos) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::FORBIDDEN_403, req, server);
    return false;
  }

  return true;
}

bool isFileExistsAndExecutable(const std::string& filePath, Response& res, const Request& req) {
  // Check if file exists and is executable
  if (!std::filesystem::exists(filePath)) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
    return false;
  }

  return true;
}

bool isFileExistsAndExecutable(const std::string& filePath, Response& res, const Request& req, const Server& server) {
  // Check if file exists and is executable
  if (!std::filesystem::exists(filePath)) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req, server);
    return false;
  }

  return true;
}

} // namespace utils
} // namespace router
