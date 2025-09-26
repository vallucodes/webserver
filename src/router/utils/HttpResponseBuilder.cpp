/**
 * @file HttpResponseBuilder.cpp
 * @brief Error response building utilities implementation
 */

#include "HttpResponseBuilder.hpp"
#include "../HttpConstants.hpp"
#include "../../response/Response.hpp"
#include "../../request/Request.hpp"
#include "Utils.hpp"
#include "FileUtils.hpp"

#include <iostream> // for std::cout, std::endl
#include <algorithm> // for std::transform

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

  // Set the response body with the error page HTML
  res.setBody(getErrorPageHtml(status));
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

std::string HttpResponseBuilder::getErrorPageHtml(int status) {
  switch (status) {
    case http::NOT_FOUND_404:
      return FileUtils::readFileToString(error_page::ERROR_PAGE_NOT_FOUND_404);
    case http::METHOD_NOT_ALLOWED_405:
      return FileUtils::readFileToString(error_page::ERROR_PAGE_METHOD_NOT_ALLOWED_405);
    case http::BAD_REQUEST_400:
      return FileUtils::readFileToString(error_page::ERROR_PAGE_BAD_REQUEST_400);
    case http::PAYLOAD_TOO_LARGE_413:
      return FileUtils::readFileToString(error_page::ERROR_PAGE_PAYLOAD_TOO_LARGE_413);
    case http::INTERNAL_SERVER_ERROR_500:
      return FileUtils::readFileToString(error_page::ERROR_PAGE_INTERNAL_SERVER_ERROR_500);
    case http::GATEWAY_TIMEOUT_504:
      return FileUtils::readFileToString(error_page::ERROR_PAGE_GATEWAY_TIMEOUT_504);
    case http::REQUEST_TIMEOUT_408:
      return FileUtils::readFileToString(error_page::ERROR_PAGE_REQUEST_TIMEOUT_408);
    default:
      // Fallback to generic 500 error for unknown status codes
      return FileUtils::readFileToString(error_page::ERROR_PAGE_INTERNAL_SERVER_ERROR_500);
  }
}

void HttpResponseBuilder::setErrorResponse(Response& res, const Request& req) {
  // Parse the status code from the request's status string
  int statusCode = parseStatusCodeFromString(std::string(req.getStatus()));

  // Use the existing method with keep-alive support
  setErrorResponse(res, statusCode, req);
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
