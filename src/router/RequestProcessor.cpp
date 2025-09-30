/**
 * @file RequestProcessor.cpp
 * @brief Implementation of RequestProcessor class
 */

#include "RequestProcessor.hpp"
#include "utils/HttpResponseBuilder.hpp"
#include "handlers/Handlers.hpp"
#include "Router.hpp"
#include "utils/StringUtils.hpp"
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
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::METHOD_NOT_ALLOWED_405, req, server);
    return;
  }

  // Execute handler if available
  if (handler) {
    if (executeHandler(handler, req, res, server)) {
      return;
    }
  }

  // Check if path exists but method is not allowed (405 Method Not Allowed)
  if (isPathExistsButMethodNotAllowed(req, server)) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::METHOD_NOT_ALLOWED_405, req, server);
    return;
  }

  // Check if path matches any configured location before fallback
  if (!isPathConfigured(req, server)) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req, server);
    return;
  }

  // Fallback: try to serve as static file (only for configured paths)
  if (tryServeAsStaticFile(req, res, method, server)) {
    return;
  }

  // All attempts failed - return 404
  router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req, server);
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
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req, server);
    return false;
  } catch (...) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req, server);
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

/** Check if path exists but method is not allowed */
bool RequestProcessor::isPathExistsButMethodNotAllowed(const Request& req, const Server& server) const {
  std::string_view path_view = req.getPath();
  std::string_view method_view = req.getMethod();

  std::string path(path_view);
  std::string method(method_view);

  // Normalize path by collapsing multiple consecutive slashes
  path = router::utils::StringUtils::normalizePath(path);

  // Find matching location for this path
  const Location* location = findLocationForPath(server, path);
  if (!location) {
    return false; // No location found, so path doesn't exist
  }

  // Check if the method is in the allowed methods for this location
  const auto& allowed_methods = location->allowed_methods;
  return std::find(allowed_methods.begin(), allowed_methods.end(), method) == allowed_methods.end();
}

/** Check if path matches any configured location */
bool RequestProcessor::isPathConfigured(const Request& req, const Server& server) const {
  std::string_view path_view = req.getPath();
  std::string path(path_view);

  // Normalize path by collapsing multiple consecutive slashes
  path = router::utils::StringUtils::normalizePath(path);

  // Check if path matches any configured location
  const Location* location = findLocationForPath(server, path);
  return location != nullptr;
}

/** Find matching location configuration for a path */
const Location* RequestProcessor::findLocationForPath(const Server& server, const std::string& path) const {
  const auto& locations = server.getLocations();
  const Location* best_match = nullptr;
  size_t best_match_length = 0;

  for (const auto& location : locations) {
    const std::string& location_path = location.location;

    // Priority 1: Exact match - highest specificity
    if (location_path == path) {
      return &location; // Return immediately for exact match
    }

    // Priority 2: Prefix match - find longest matching prefix
    // Example: location "/admin" matches path "/admin/page"
    if (path.length() > location_path.length() &&
        path.substr(0, location_path.length()) == location_path) {

      // Additional validation for prefix matching
      bool is_valid_prefix_match = false;

      // Case 1: Path continues after location_path with a slash
      if (path[location_path.length()] == '/') {
        is_valid_prefix_match = true;
      }
      // Case 2: Special case for root "/" - match files in root directory
      else if (location_path == "/" && path.length() > 1) {
        // Root should match files in root directory like "/index.html", "/favicon.ico"
        // but not subdirectories like "/uploads/file.txt"
        std::string remaining = path.substr(1); // Remove leading "/"
        if (remaining.find('/') == std::string::npos) {
          is_valid_prefix_match = true; // File in root directory
        }
      }

      if (is_valid_prefix_match) {
        // Keep track of the longest (most specific) prefix match
        if (location_path.length() > best_match_length) {
          best_match = &location;
          best_match_length = location_path.length();
        }
      }
    }
    // Priority 3: Extension-based matching
    // Example: location ".py" matches path "/script.py"
    else if (!location_path.empty() && location_path[0] == '.' && path.length() > location_path.length()) {
      if (path.substr(path.length() - location_path.length()) == location_path) {
        // Extension matches have higher priority than prefix matches
        if (location_path.length() > best_match_length) {
          best_match = &location;
          best_match_length = location_path.length();
        }
      }
    }
  }

  return best_match; // Return best match or nullptr
}
