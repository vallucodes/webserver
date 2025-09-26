/**
 * @file HttpConstants.hpp
 * @brief HTTP Protocol Constants
 */

#pragma once

#include <string> // for std::string
#include <vector> // for std::vector

/**
 * @namespace http
 * @brief HTTP protocol constants
 */
namespace http {
  // HTTP Status Messages (Complete Status Lines)
  const std::string STATUS_OK_200 = "200 OK";
  const std::string STATUS_CREATED_201 = "201 Created";
  const std::string STATUS_NO_CONTENT_204 = "204 No Content";
  const std::string STATUS_FOUND_302 = "302 Found";
  const std::string STATUS_NOT_FOUND_404 = "404 Not Found";
  const std::string STATUS_FORBIDDEN_403 = "403 Forbidden";
  const std::string STATUS_METHOD_NOT_ALLOWED_405 = "405 Method Not Allowed";
  const std::string STATUS_BAD_REQUEST_400 = "400 Bad Request";
  const std::string STATUS_PAYLOAD_TOO_LARGE_413 = "413 Payload Too Large";
  const std::string STATUS_INTERNAL_SERVER_ERROR_500 = "500 Internal Server Error";
  const std::string STATUS_GATEWAY_TIMEOUT_504 = "504 Gateway Timeout";
  const std::string STATUS_REQUEST_TIMEOUT_408 = "408 Request Timeout";

  // HTTP Status Codes (Numeric Values)
  const int OK_200 = 200;
  const int CREATED_201 = 201;
  const int NO_CONTENT_204 = 204;
  const int FOUND_302 = 302;
  const int NOT_FOUND_404 = 404;
  const int FORBIDDEN_403 = 403;
  const int METHOD_NOT_ALLOWED_405 = 405;
  const int BAD_REQUEST_400 = 400;
  const int PAYLOAD_TOO_LARGE_413 = 413;
  const int INTERNAL_SERVER_ERROR_500 = 500;
  const int GATEWAY_TIMEOUT_504 = 504;
  const int REQUEST_TIMEOUT_408 = 408;

  // HTTP Methods
  const std::string GET = "GET";
  const std::string POST = "POST";
  const std::string DELETE = "DELETE";


  // Common HTTP Headers
  const std::string CONTENT_TYPE = "Content-Type";
  const std::string CONTENT_LENGTH = "Content-Length";
  const std::string CONNECTION = "Connection";
  const std::string LOCATION = "Location";
  const std::string USER_AGENT = "User-Agent";
  const std::string ACCEPT = "Accept";
  const std::string HOST = "Host";
  const std::string ALLOW = "Allow";

  // Connection Values
  const std::string CONNECTION_CLOSE = "close";
  const std::string CONNECTION_KEEP_ALIVE = "keep-alive";

  // Content Types
  const std::string CONTENT_TYPE_HTML = "text/html";
  const std::string CONTENT_TYPE_TEXT = "text/plain";
}

/**
 * @namespace error_page
 * @brief Error page file paths
 */
namespace error_page {
  const std::string ERROR_PAGE_NOT_FOUND_404 = "www/errors/not_found_404.html";
  const std::string ERROR_PAGE_METHOD_NOT_ALLOWED_405 = "www/errors/method_not_allowed_405.html";
  const std::string ERROR_PAGE_BAD_REQUEST_400 = "www/errors/bad_request_400.html";
  const std::string ERROR_PAGE_PAYLOAD_TOO_LARGE_413 = "www/errors/internal_server_error_500.html";
  const std::string ERROR_PAGE_INTERNAL_SERVER_ERROR_500 = "www/errors/internal_server_error_500.html";
  const std::string ERROR_PAGE_GATEWAY_TIMEOUT_504 = "www/errors/gateway_timeout_504.html";
  const std::string ERROR_PAGE_REQUEST_TIMEOUT_408 = "www/errors/request_timeout_408.html";
}

/**
 * @namespace page
 * @brief Web server content constants
 */
namespace page {
  // Directory Structure
  const std::string WWW = "www";
  const std::string ERRORS_DIR = "www/errors";
  const std::string UPLOADS_DIR = "www/uploads";

  // Standard Paths
  const std::string ROOT_HTML = "/";
  const std::string INDEX_HTML_PATH = "/index.html";
  const std::string INDEX_HTML = "www/index.html";

  // Upload-related Pages
  const std::string UPLOAD_HTML = "www/upload.html";

  // Default Files
  const std::vector<std::string> DEFAULT_INDEX_FILES = {
    "index.html"
  };
}
