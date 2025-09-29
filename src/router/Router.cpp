/**
 * @file Router.cpp
 * @brief Implementation of HTTP Router class
 */

#include "../../inc/webserv.hpp"
#include "Router.hpp"
#include "utils/HttpResponseBuilder.hpp"
#include "utils/StringUtils.hpp"
#include "HttpConstants.hpp"
#include "handlers/Handlers.hpp"
#include <algorithm> // for std::find

using namespace router::utils;

/** Default constructor */
Router::Router() {}

/** Destructor */
Router::~Router() {}

// ========================= ROUTER SETUP =========================

/** Initialize router with server configurations */
void Router::setupRouter(const std::vector<Server>& configs) {
  // Clear existing routes to start fresh
  _routes.clear();

  // Iterate through each server configuration
  for (size_t i = 0; i < configs.size(); ++i) {
    const Server& server = configs[i];

    // Get server root directory for resolving relative paths
    std::string server_root = server.getRoot();

    // Process each location block in the server configuration
    for (const auto& location : server.getLocations()) {
      std::string location_path = location.location;

      // Handle different location types:
      // 1. Root location "/" - matches all requests not handled elsewhere
      // 2. Exact paths like "/favicon.ico" - matches specific files
      // 3. Extension-based locations like ".py" - matches file extensions
      // 4. Prefix paths like "/upload" - matches directory prefixes

      // Register routes for each allowed HTTP method in this location
      for (const auto& method : location.allowed_methods) {
        // Choose the appropriate handler based on location configuration and method
        Handler handler;

        // Handler selection logic based on location properties:

        if (!location.return_url.empty()) {
          // Redirect location: return_url is configured
          // Use redirect handler for all HTTP methods
          handler = [&server](const Request& req, Response& res, const Server& /* srv */) {
            redirect(req, res, server);
          };
        } else if (!location.cgi_path.empty() && !location.cgi_ext.empty()) {
          // CGI location: Both CGI path and extension are configured
          // Use CGI handler for script execution (supports any HTTP method)
          handler = [&server](const Request& req, Response& res, const Server& /* srv */) {
            cgi(req, res, server);
          };
        } else if (method == http::POST && !location.upload_path.empty()) {
          // POST request to upload location: Handle file uploads
          handler = [&server](const Request& req, Response& res, const Server& /* srv */) {
            post(req, res, server);
          };
        } else if (method == http::DELETE && !location.upload_path.empty()) {
          // DELETE request from upload location: Handle file deletions
          handler = [&server](const Request& req, Response& res, const Server& /* srv */) {
            del(req, res, server);
          };
        } else {
          // Default handler for other methods or configurations
          handler = [&server](const Request& req, Response& res, const Server& /* srv */) {
            get(req, res, server);
          };
        }

        // Register the route in the routing table using server ID instead of port
        addRoute(server.getId(), method, location_path, handler);
      }
    }
  }
  // Display all registered routes for verification
  listRoutes();
}

// ========================= ROUTES REGISTRATION =========================

/** Register a new route */
void Router::addRoute(int server_id, std::string_view method, std::string_view path, Handler handler) {
  _routes[server_id][std::string(path)][std::string(method)] = std::move(handler);
}

// ========================= REQUEST HANDLING =========================

/** Find handler for server/method/path */
const Router::Handler* Router::findHandler(int server_id, const std::string& method, const std::string& path) const {
  // Step 1: Find the server in our routing table
  auto server_it = _routes.find(server_id);
  if (server_it == _routes.end()) {
    return nullptr; // Server not found in routing table
  }

  const auto& server_routes = server_it->second;

  // Step 2: Try exact path match first (highest priority)
  auto path_it = server_routes.find(path);
  if (path_it != server_routes.end()) {
    auto method_it = path_it->second.find(method);
    if (method_it != path_it->second.end()) {
      return &method_it->second; // Exact match found
    }
  }

  // Step 3: Find the best advanced match (longest prefix or extension match)
  const Handler* best_handler = nullptr;
  size_t best_match_length = 0;
  bool is_extension_match = false;

  for (const auto& route_pair : server_routes) {
    const std::string& route_path = route_pair.first;

    // Check if this route has the requested method
    auto method_it = route_pair.second.find(method);
    if (method_it == route_pair.second.end()) {
      continue;
    }

    // Strategy A: Extension-based matching
    // Example: Route ".py" should match "/cgi-bin/script.py" or "/script.py"
    if (!route_path.empty() && route_path[0] == '.' && path.length() > route_path.length()) {
      if (path.substr(path.length() - route_path.length()) == route_path) {
        // Extension matches have higher priority than prefix matches
        if (!is_extension_match || route_path.length() > best_match_length) {
          best_handler = &method_it->second;
          best_match_length = route_path.length();
          is_extension_match = true;
        }
      }
    }
    // Strategy B: Prefix-based matching
    // Example: Route "/uploads" should match "/uploads/file.txt"
    else if (!route_path.empty() &&
             path.length() >= route_path.length() &&
             path.substr(0, route_path.length()) == route_path &&
             (route_path.back() == '/' || path.length() == route_path.length() || path[route_path.length()] == '/')) {
      // For prefix matches, prefer longer routes (more specific)
      if (!is_extension_match && route_path.length() > best_match_length) {
        best_handler = &method_it->second;
        best_match_length = route_path.length();
      }
    }
  }

  return best_handler;
}


/** Find matching location configuration */
const Location* Router::findLocation(const Server& server, const std::string& path) const {
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
        path.substr(0, location_path.length()) == location_path &&
        (location_path.back() == '/' || path[location_path.length()] == '/')) {
      // Keep track of the longest (most specific) prefix match
      if (location_path.length() > best_match_length) {
        best_match = &location;
        best_match_length = location_path.length();
      }
    }
  }

  return best_match; // Return best prefix match or nullptr
}

/**
 * @brief Generate HTML error page
 * @param status HTTP status code
 * @return HTML content string
 */
// Now using router::utils::HttpResponseBuilder::getErrorPageHtml instead

/**
 * @brief Set up error response
 * @param res Response object
 * @param status HTTP status code
 */
// Now using router::utils::HttpResponseBuilder::setErrorResponse instead

/** Process HTTP request and route to handler */
void Router::handleRequest(const Server& server, const Request& req, Response& res) const {
  // Check if parser found errors
  if (req.getError()) {
    // Parse the status code from the request's status string
    int statusCode = router::utils::HttpResponseBuilder::parseStatusCodeFromString(std::string(req.getStatus()));
    router::utils::HttpResponseBuilder::setErrorResponse(res, statusCode, req);
    return;
  }
  // Extract method and path from the request
  std::string_view method_view = req.getMethod();
  std::string_view path_view = req.getPath();

  std::string method(method_view);
  std::string path(path_view);

  // Normalize path by collapsing multiple consecutive slashes
  path = router::utils::StringUtils::normalizePath(path);

  // Find the appropriate handler for this request
  const Handler* handler = findHandler(server.getId(), method, path);

  // Delegate to RequestProcessor for execution and fallback handling
  _requestProcessor.processRequest(req, handler, res, server);
}

// ========================= HELPERS =========================

/** List all registered routes (debug function) */
void Router::listRoutes() const {

  std::cout << "=== Available routes: ===" << std::endl;
  for (const auto& server_pair : _routes) {
    int server_id = server_pair.first;
    std::cout << "Server ID: " << server_id << std::endl;

    for (const auto& path_pair : server_pair.second) {
      const std::string& path = path_pair.first;
      std::cout << "  " << path << " -> ";
      for (const auto& method_pair : path_pair.second) {
        std::cout << method_pair.first << " ";
      }
      std::cout << std::endl;
    }
  }
  std::cout << "=========================\n" << std::endl;
}

void Router::requestTimeOut() {
  // Log the 408 Request Timeout event
  std::cout << "408 Request Timeout - Client request timed out" << std::endl;
}

