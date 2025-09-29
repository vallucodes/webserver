/**
 * @file HttpResponseBuilder.cpp
 * @brief Error response building utilities implementation
 */

#include "HttpResponseBuilder.hpp"
#include "../HttpConstants.hpp"
#include "../../response/Response.hpp"
#include "../../request/Request.hpp"
#include "../../server/Server.hpp"
#include "Utils.hpp"
#include "FileUtils.hpp"

#include <iostream> // for std::cout, std::endl
#include <algorithm> // for std::transform
#include <sstream> // for std::ostringstream

namespace router {
namespace utils {

void HttpResponseBuilder::setErrorResponse(Response& res, int status, const Request& req) {
  // Set the HTTP status line based on the error code
  if (status == http::NOT_FOUND_404) {
    res.setStatus(http::STATUS_NOT_FOUND_404);
  } else if (status == http::METHOD_NOT_ALLOWED_405) {
    res.setStatus(http::STATUS_METHOD_NOT_ALLOWED_405);
  } else if (status == http::BAD_REQUEST_400) {
    res.setStatus(http::STATUS_BAD_REQUEST_400);
  } else if (status == http::PAYLOAD_TOO_LARGE_413) {
    res.setStatus(http::STATUS_PAYLOAD_TOO_LARGE_413);
  } else if (status == http::FORBIDDEN_403) {
    res.setStatus(http::STATUS_FORBIDDEN_403);
  } else if (status == http::INTERNAL_SERVER_ERROR_500) {
    res.setStatus(http::STATUS_INTERNAL_SERVER_ERROR_500);
  } else if (status == http::GATEWAY_TIMEOUT_504) {
    res.setStatus(http::STATUS_GATEWAY_TIMEOUT_504);
  } else if (status == http::REQUEST_TIMEOUT_408) {
    res.setStatus(http::STATUS_REQUEST_TIMEOUT_408);
  } else {
    res.setStatus(http::STATUS_INTERNAL_SERVER_ERROR_500);
  }

  // Set standard headers for HTML error responses
  res.setHeaders(http::CONTENT_TYPE, http::CONTENT_TYPE_HTML);
  res.setHeaders(http::CONTENT_LENGTH, std::to_string(getErrorPageHtml(status).length()));

  // Set connection header based on keep-alive logic
  if (router::utils::shouldKeepAlive(req)) {
    res.setHeaders(http::CONNECTION, http::CONNECTION_KEEP_ALIVE);
  } else {
    res.setHeaders(http::CONNECTION, http::CONNECTION_CLOSE);
  }

  // Set the response body with the default error page HTML
  res.setBody(getErrorPageHtml(status));
}

void HttpResponseBuilder::setErrorResponse(Response& res, int status, const Request& req, const Server& server) {
  // Set the HTTP status line based on the error code
  if (status == http::NOT_FOUND_404) {
    res.setStatus(http::STATUS_NOT_FOUND_404);
  } else if (status == http::METHOD_NOT_ALLOWED_405) {
    res.setStatus(http::STATUS_METHOD_NOT_ALLOWED_405);
  } else if (status == http::BAD_REQUEST_400) {
    res.setStatus(http::STATUS_BAD_REQUEST_400);
  } else if (status == http::PAYLOAD_TOO_LARGE_413) {
    res.setStatus(http::STATUS_PAYLOAD_TOO_LARGE_413);
  } else if (status == http::FORBIDDEN_403) {
    res.setStatus(http::STATUS_FORBIDDEN_403);
  } else if (status == http::INTERNAL_SERVER_ERROR_500) {
    res.setStatus(http::STATUS_INTERNAL_SERVER_ERROR_500);
  } else if (status == http::GATEWAY_TIMEOUT_504) {
    res.setStatus(http::STATUS_GATEWAY_TIMEOUT_504);
  } else if (status == http::REQUEST_TIMEOUT_408) {
    res.setStatus(http::STATUS_REQUEST_TIMEOUT_408);
  } else {
    res.setStatus(http::STATUS_INTERNAL_SERVER_ERROR_500);
  }

  // Get error page HTML (will use server-level custom pages if configured)
  std::string errorPageHtml = getErrorPageHtml(status, server);

  // Set standard headers for HTML error responses
  res.setHeaders(http::CONTENT_TYPE, http::CONTENT_TYPE_HTML);
  res.setHeaders(http::CONTENT_LENGTH, std::to_string(errorPageHtml.length()));

  // Set connection header based on keep-alive logic
  if (router::utils::shouldKeepAlive(req)) {
    res.setHeaders(http::CONNECTION, http::CONNECTION_KEEP_ALIVE);
  } else {
    res.setHeaders(http::CONNECTION, http::CONNECTION_CLOSE);
  }

  // Set the response body with the error page HTML (custom or default)
  res.setBody(errorPageHtml);
}

void HttpResponseBuilder::setSuccessResponse(Response& res, const std::string& content, const std::string& contentType, const Request& req) {
  res.setStatus(http::STATUS_OK_200);
  res.setHeaders(http::CONTENT_TYPE, contentType);
  res.setHeaders(http::CONTENT_LENGTH, std::to_string(content.length()));

  // Set connection header based on keep-alive logic
  if (router::utils::shouldKeepAlive(req)) {
    res.setHeaders(http::CONNECTION, http::CONNECTION_KEEP_ALIVE);
  } else {
    res.setHeaders(http::CONNECTION, http::CONNECTION_CLOSE);
  }

  res.setBody(content);
}

void HttpResponseBuilder::setCreatedResponse(Response& res, const std::string& content, const std::string& contentType, const Request& req) {
  res.setStatus(http::STATUS_CREATED_201);
  res.setHeaders(http::CONTENT_TYPE, contentType);
  res.setHeaders(http::CONTENT_LENGTH, std::to_string(content.length()));

  // Set connection header based on keep-alive logic
  if (router::utils::shouldKeepAlive(req)) {
    res.setHeaders(http::CONNECTION, http::CONNECTION_KEEP_ALIVE);
  } else {
    res.setHeaders(http::CONNECTION, http::CONNECTION_CLOSE);
  }

  res.setBody(content);
}

void HttpResponseBuilder::setNoContentResponse(Response& res, const Request& req) {
  res.setStatus(http::STATUS_NO_CONTENT_204);
  res.setHeaders(http::CONTENT_LENGTH, "0");

  // Set connection header based on keep-alive logic
  if (router::utils::shouldKeepAlive(req)) {
    res.setHeaders(http::CONNECTION, http::CONNECTION_KEEP_ALIVE);
  } else {
    res.setHeaders(http::CONNECTION, http::CONNECTION_CLOSE);
  }

  res.setBody(""); // Empty body for 204 No Content
}

void HttpResponseBuilder::setMethodNotAllowedResponse(Response& res, const std::vector<std::string>& allowedMethods, const Request& req) {
  // Set the HTTP status line
  res.setStatus(http::STATUS_METHOD_NOT_ALLOWED_405);

  // Set standard headers for HTML error responses
  res.setHeaders(http::CONTENT_TYPE, http::CONTENT_TYPE_HTML);

  // Build the Allow header from the allowed methods list
  std::string allowHeader;
  for (size_t i = 0; i < allowedMethods.size(); ++i) {
    if (i > 0) {
      allowHeader += ", ";
    }
    allowHeader += allowedMethods[i];
  }
  res.setHeaders(http::ALLOW, allowHeader);

  // Set content length
  std::string errorHtml = getErrorPageHtml(http::METHOD_NOT_ALLOWED_405);
  res.setHeaders(http::CONTENT_LENGTH, std::to_string(errorHtml.length()));

  // Set connection header based on keep-alive logic
  if (router::utils::shouldKeepAlive(req)) {
    res.setHeaders(http::CONNECTION, http::CONNECTION_KEEP_ALIVE);
  } else {
    res.setHeaders(http::CONNECTION, http::CONNECTION_CLOSE);
  }

  // Set the response body with the error page HTML
  res.setBody(errorHtml);
}

std::string HttpResponseBuilder::makeDefaultErrorPage(int code, const std::string& reason) {
  std::ostringstream oss;
  oss << "<html>\n<head><title>" << code << " " << reason << "</title></head>\n"
      << "<body>\n<center><h1>" << code << " " << reason << "</h1></center>\n"
      << "</body>\n</html>\n";
  return oss.str();
}

std::string HttpResponseBuilder::getErrorPageHtml(int status) {
  switch (status) {
    case http::BAD_REQUEST_400:
      return makeDefaultErrorPage(400, "Bad Request");
    case http::FORBIDDEN_403:
      return makeDefaultErrorPage(403, "Forbidden");
    case http::NOT_FOUND_404:
      return makeDefaultErrorPage(404, "Not Found");
    case http::METHOD_NOT_ALLOWED_405:
      return makeDefaultErrorPage(405, "Method Not Allowed");
    case http::PAYLOAD_TOO_LARGE_413:
      return makeDefaultErrorPage(413, "Payload Too Large");
    case http::INTERNAL_SERVER_ERROR_500:
      return makeDefaultErrorPage(500, "Internal Server Error");
    case http::GATEWAY_TIMEOUT_504:
      return makeDefaultErrorPage(504, "Gateway Timeout");
    case http::REQUEST_TIMEOUT_408:
      return makeDefaultErrorPage(408, "Request Timeout");
    default:
      // Fallback to generic 500 error for unknown status codes
      return makeDefaultErrorPage(500, "Internal Server Error");
  }
}

std::string HttpResponseBuilder::getErrorPageHtml(int status, const Server& server) {
  // Check if server has custom error page for this status
  const std::map<int, std::string>& errorPages = server.getErrorPages();
  auto it = errorPages.find(status);

  if (it != errorPages.end()) {
    // Server has custom error page configured
    std::string customErrorPath = it->second;

    // Build full path using server root
    std::string fullPath = server.getRoot() + "/" + customErrorPath;

    // Try to read the custom error page file
    try {
      std::string customErrorContent = FileUtils::readFileToString(fullPath);
      if (!customErrorContent.empty()) {
        return customErrorContent;
      }
    } catch (const std::exception&) {
      // File doesn't exist or can't be read, fall back to default
    }
  }

  // No custom error page or file doesn't exist, use default
  return getErrorPageHtml(status);
}

int HttpResponseBuilder::parseStatusCodeFromString(const std::string& statusString) {
  // Extract the numeric status code from strings like "400 Bad Request"
  if (statusString.find("400") != std::string::npos) {
    return http::BAD_REQUEST_400;
  } else if (statusString.find("403") != std::string::npos) {
    return http::FORBIDDEN_403;
  } else if (statusString.find("404") != std::string::npos) {
    return http::NOT_FOUND_404;
  } else if (statusString.find("405") != std::string::npos) {
    return http::METHOD_NOT_ALLOWED_405;
  } else if (statusString.find("413") != std::string::npos) {
    return http::PAYLOAD_TOO_LARGE_413;
  } else if (statusString.find("500") != std::string::npos) {
    return http::INTERNAL_SERVER_ERROR_500;
  } else if (statusString.find("504") != std::string::npos) {
    return http::GATEWAY_TIMEOUT_504;
  } else if (statusString.find("408") != std::string::npos) {
    return http::REQUEST_TIMEOUT_408;
  } else {
    // Default to 400 Bad Request for unknown status codes
    return http::BAD_REQUEST_400;
  }
}

} // namespace utils
} // namespace router
