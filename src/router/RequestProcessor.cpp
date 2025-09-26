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
                                      Response& res, const Server& server) const {
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
    if (executeHandler(handler, req, res, server)) {
      return;
    }
  }


  // Fallback: try to serve as static file
  if (tryServeAsStaticFile(req, res, method, server)) {
    return;
  }

  // All attempts failed - return 404
  router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
}

/** Execute handler */
bool RequestProcessor::executeHandler(const Handler* handler,
                                      const Request& req, Response& res,
                                      const Server& server) const {
  if (!handler) {
    return false;
  }
  try {
    // Execute the handler with the server
    (*handler)(req, res, server);
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
                                            const std::string& method, const Server& server) const {
  // Only handle GET requests for static files
  if (method != http::GET) {
    return false;
  }

  try {
    // Attempt to serve using the GET handler
    get(req, res, server);
    return true;
  } catch (...) {
    // Static file serving failed
    return false;
  }
}

