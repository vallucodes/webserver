/**
 * @file Router.cpp
 * @brief Implementation of HTTP Router class
 */

#include "../../inc/webserv.hpp"
#include "Router.hpp"
#include "utils/ErrorResponseBuilder.hpp"
#include "HttpConstants.hpp"
#include "handlers/Handlers.hpp"
#include <algorithm>


/**
 * @brief Default constructor
 */
Router::Router() {}

/**
 * @brief Destructor
 */
Router::~Router() {}

/**
 * @brief List all registered routes (debug function)
 */
void Router::listRoutes() const {

    std::cout << "=== Available routes: ===" << std::endl;
    for (const auto& server_pair : _routes) {
        const std::string& server_name = server_pair.first;
        std::cout << "Server: " << server_name << std::endl;

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

/**
 * @brief Initialize router with server configurations
 * @param configs Vector of Server configuration objects
 */
void Router::setupRouter(const std::vector<Server>& configs) {
    // Clear existing routes to start fresh
    _routes.clear();

    // Iterate through each server configuration
    for (const auto& server : configs) {
        // Get server root directory for resolving relative paths
        // std::string server_root = server.getRoot(); // maybe needed in the future

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
                    handler = redirect;
                } else if (!location.cgi_path.empty() && !location.cgi_ext.empty()) {
                    // CGI location: Both CGI path and extension are configured
                    // Use CGI handler for script execution (supports any HTTP method)
                    handler = cgi;
                } else if (method == "POST" && !location.upload_path.empty()) {
                    // POST request to upload location: Handle file uploads
                    handler = post;
                } else if (method == "DELETE" && !location.upload_path.empty()) {
                    // DELETE request from upload location: Handle file deletions
                    handler = del;
                // if HEAD method needed
                // } else if (method == "GET" || method == "HEAD") {
                //     // GET/HEAD requests: Handle static file serving
                //     // HEAD is identical to GET but without response body
                //     handler = get;
                } else {
                    // Default handler for other methods or configurations
                    handler = get;
                }

                // Register the route in the routing table
                addRoute(server.getName(), method, location_path, handler);

                // if HEAD method needed
                // HTTP convention: Automatically add HEAD support for GET routes
                // HEAD requests should return the same headers as GET but no body
                // Only add if HEAD is not already explicitly allowed in location config
                // if (method == "GET" && std::find(location.allowed_methods.begin(),
                //     location.allowed_methods.end(), "HEAD") == location.allowed_methods.end()) {
                //     addRoute(server.getName(), "HEAD", location_path, handler);
                // }
            }
        }
    }
    // Display all registered routes for verification
    listRoutes();
}

/**
 * @brief Register a new route
 * @param server_name Server name
 * @param method HTTP method
 * @param path URL path pattern
 * @param handler Handler function
 */
void Router::addRoute(std::string_view server_name, std::string_view method, std::string_view path, Handler handler) {
    _routes[std::string(server_name)][std::string(path)][std::string(method)] = std::move(handler);
}

/**
 * @brief Find handler for server/method/path
 * @param server_name Server name
 * @param method HTTP method
 * @param path URL path
 * @return Handler function or nullptr
 */
const Router::Handler* Router::findHandler(const std::string& server_name, const std::string& method, const std::string& path) const {
    // Step 1: Find the server in our routing table
    auto server_it = _routes.find(server_name);
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


/**
 * @brief Find matching location configuration
 * @param server Server configuration
 * @param path URL path
 * @return Location or nullptr
 */
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
// Now using ErrorResponseBuilder::getErrorPageHtml instead

/**
 * @brief Set up error response
 * @param res Response object
 * @param status HTTP status code
 */
// Now using ErrorResponseBuilder::setErrorResponse instead

/**
 * @brief Process HTTP request and route to handler
 * @param server Server configuration
 * @param req HTTP request
 * @param res Response object
 */
void Router::handleRequest(const Server& server, const Request& req, Response& res) const {
    // Extract method and path from the request
    std::string_view method_view = req.getMethod();
    std::string_view path_view = req.getPath();

    std::string method(method_view);
    std::string path(path_view);

    // Find the appropriate handler for this request
    const Handler* handler = findHandler(server.getName(), method, path);

    // Find the matching location configuration for this request
    const Location* location = findLocation(server, path);

    // Delegate to RequestProcessor for execution and fallback handling
    _requestProcessor.processRequest(server, req, handler, res, location);
}




