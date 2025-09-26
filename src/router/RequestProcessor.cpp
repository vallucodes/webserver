/**
 * @file RequestProcessor.cpp
 * @brief Implementation of RequestProcessor class
 */

#include "RequestProcessor.hpp"
#include "utils/HttpResponseBuilder.hpp"
#include "handlers/Handlers.hpp"
#include <iostream> // for std::cout, std::endl
#include <algorithm> // for std::find

using namespace router::utils;

/** Default constructor */
RequestProcessor::RequestProcessor() {
  // Constructor can be used for initialization if needed
}

/** Destructor */
RequestProcessor::~RequestProcessor() {
  // Destructor can be used for cleanup if needed
}

/** Process HTTP request */
void RequestProcessor::processRequest(const Request& req, const Handler* handler,
                                      Response& res, const Location* location) const {
  // Extract and validate request components
  std::string_view method_view = req.getMethod();
  std::string_view path_view = req.getPath();

  std::string method(method_view);
  std::string path(path_view);

  // Validate HTTP method - return 405 Method Not Allowed for unsupported methods
  if (method != http::GET && method != http::POST && method != http::DELETE) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::METHOD_NOT_ALLOWED_405, req);
    return;
  }

  // Execute handler if available
  if (handler) {
    if (executeHandler(handler, req, res, location)) {
      return;
    }
  }

  // Check if method is allowed for this location
  if (location) {
    const auto& allowedMethods = location->allowed_methods;
    bool methodAllowed = std::find(allowedMethods.begin(), allowedMethods.end(), method) != allowedMethods.end();
    if (!methodAllowed) {
      router::utils::HttpResponseBuilder::setErrorResponse(res, http::METHOD_NOT_ALLOWED_405, req);
      return;
    }
  }

  // Fallback: try to serve as static file
  if (tryServeAsStaticFile(req, res, method)) {
    return;
  }

  // All attempts failed - return 404
  router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
}

/** Execute handler */
bool RequestProcessor::executeHandler(const Handler* handler,
                                      const Request& req, Response& res,
                                      const Location* location) const {
  if (!handler) {
    return false;
  }
  try {
    // Execute the handler with the passed location
    (*handler)(req, res, location);
    return true;
  } catch (const std::exception& e) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
    return false;
  } catch (...) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
    return false;
  }
}

/** Serve static file */
bool RequestProcessor::tryServeAsStaticFile(const Request& req, Response& res,
                                            const std::string& method) const {
  // Only handle GET requests for static files
  if (method != http::GET) {
    return false;
  }

  try {
    // Attempt to serve using the GET handler
    get(req, res, nullptr);
    return true;
  } catch (...) {
    // Static file serving failed
    return false;
  }
}

/** Generate error response */
void RequestProcessor::generateErrorResponse(Response& res, int status,
                                             const std::string& message) const {
  // Set appropriate status based on error code
  switch (status) {
    case http::BAD_REQUEST_400:
      res.setStatus(http::STATUS_BAD_REQUEST_400);
      break;
    case http::NOT_FOUND_404:
      res.setStatus(http::STATUS_NOT_FOUND_404);
      break;
    case http::METHOD_NOT_ALLOWED_405:
      res.setStatus(http::STATUS_METHOD_NOT_ALLOWED_405);
      break;
    case http::INTERNAL_SERVER_ERROR_500:
    default:
      res.setStatus(http::STATUS_INTERNAL_SERVER_ERROR_500);
      break;
  }

  // Set common headers
  res.setHeaders(http::CONTENT_TYPE, http::CONTENT_TYPE_HTML);
  // Default to keep-alive for HTTP/1.1 compatibility
  res.setHeaders(http::CONNECTION, http::CONNECTION_KEEP_ALIVE);

  // For now, set a simple error message
  // In a real implementation, this would load custom error pages
  std::string errorBody = "<html><body><h1>Error " +
                          std::to_string(status) + "</h1>";
  if (!message.empty()) {
    errorBody += "<p>" + message + "</p>";
  }
  errorBody += "</body></html>";

  res.setHeaders(http::CONTENT_LENGTH, std::to_string(errorBody.length()));
  res.setBody(errorBody);
}
