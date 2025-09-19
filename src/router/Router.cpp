/**
 * @file Router.cpp
 * @brief Implementation of the HTTP Router class for request routing and handling
 *
 * This file contains the implementation of the Router class which provides:
 * - Route registration and management for multiple servers
 * - HTTP request routing based on server name, method, and path
 * - Location-based configuration matching
 * - Error response generation and handling
 * - Support for CGI scripts, file uploads, and static file serving
 *
 * The router implements sophisticated path matching algorithms including:
 * - Exact path matching
 * - Prefix-based matching (e.g., "/admin" matches "/admin/users")
 * - Extension-based matching (e.g., ".py" matches "/cgi/script.py")
 * - Automatic HEAD method support for GET routes
 *
 * Route lookup is performed hierarchically: server -> path -> method
 * for efficient O(log n) performance using std::map containers.
 */

#include "../../inc/webserv.hpp"
#include "Router.hpp"
#include "utils/ErrorResponseBuilder.hpp"
#include "HttpConstants.hpp"
#include "handlers/Handlers.hpp"
#include <algorithm>


/**
 * @brief Default constructor for Router
 *
 * Initializes an empty router with no routes configured.
 */
Router::Router() {}

/**
 * @brief Destructor for Router
 *
 * Cleans up router resources. Since std::map handles its own memory
 * management, no explicit cleanup is required.
 */
Router::~Router() {}

/**
 * @brief Debug method to list all registered routes to console
 *
 * This method provides a comprehensive view of all registered routes
 * organized by server name. For each server, it displays:
 * - Server name
 * - Route paths and their supported HTTP methods
 *
 * Output format:
 * Server: server_name
 *   /path -> GET POST DELETE
 *   /another -> GET HEAD
 *
 * Useful for debugging routing configuration and verifying
 * that routes are registered correctly.
 */
void Router::listRoutes() const {
    std::cout << "[DEBUG]: Available routes:" << std::endl;
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
    std::cout << "[DEBUG]: ===  End of Available routes ===\n" << std::endl;
}

/**
 * @brief Initialize the router with routes based on server configurations
 *
 * This method processes all server configurations and creates appropriate routes
 * for each location defined in each server. It implements the core routing logic:
 *
 * 1. Handler Selection Logic:
 *    - CGI locations: Use CGI handler for script execution
 *    - POST + upload_path: Use POST handler for file uploads
 *    - DELETE + upload_path: Use DELETE handler for file deletion
 *    - GET/HEAD: Use GET handler for static file serving
 *
 * 2. Route Registration:
 *    - Registers routes for each allowed HTTP method per location
 *    - Automatically adds HEAD support for GET routes (HTTP convention)
 *    - Organizes routes hierarchically: server -> path -> method
 *
 * 3. Location Types Supported:
 *    - Root location ("/"): Default server behavior
 *    - Exact paths ("/favicon.ico"): Specific file handling
 *    - Extension-based (".py"): CGI script matching
 *    - Prefix paths ("/upload/"): Directory-based routing
 *
 * @param configs Vector of Server configuration objects
 */
void Router::setupRouter(const std::vector<Server>& configs) {
    // Clear existing routes to start fresh
    _routes.clear();

    // Iterate through each server configuration
    for (const auto& server : configs) {
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
                } else if (method == "GET" || method == "HEAD") {
                    // GET/HEAD requests: Handle static file serving
                    // HEAD is identical to GET but without response body
                    handler = get;
                } else {
                    // Default handler for other methods or configurations
                    handler = get;
                }

                // Register the route in the routing table
                addRoute(server.getName(), method, location_path, handler);

                // HTTP convention: Automatically add HEAD support for GET routes
                // HEAD requests should return the same headers as GET but no body
                // Only add if HEAD is not already explicitly allowed in location config
                if (method == "GET" && std::find(location.allowed_methods.begin(),
                    location.allowed_methods.end(), "HEAD") == location.allowed_methods.end()) {
                    addRoute(server.getName(), "HEAD", location_path, handler);
                }
            }
        }
    }

    // Debug: Display all registered routes for verification
    listRoutes();
}

/**
 * @brief Register a new route with specific HTTP method and path for a server
 *
 * This method adds a route mapping to the internal routing table.
 * Routes are stored hierarchically using std::map for efficient lookup:
 *
 * _routes[server_name][path][method] = handler_function
 *
 * The use of std::move ensures efficient transfer of the handler function
 * without unnecessary copying.
 *
 * @param server_name Name of the server this route belongs to
 * @param method HTTP method (GET, POST, DELETE, etc.)
 * @param path URL path pattern to match
 * @param handler Function to handle requests for this route
 */
void Router::addRoute(std::string_view server_name, std::string_view method, std::string_view path, Handler handler) {
    _routes[std::string(server_name)][std::string(path)][std::string(method)] = std::move(handler);
}

/**
 * @brief Find the handler function for a given server, method and path
 *
 * This method implements sophisticated route matching with multiple strategies:
 *
 * 1. Exact Match: Direct path equality (highest priority)
 * 2. Extension Match: File extension matching (e.g., ".py" routes match "*.py" files)
 * 3. Prefix Match: Directory prefix matching (e.g., "/uploads" matches "/uploads/file.txt")
 *
 * Matching Priority (most specific first):
 * - Exact path match: "/exact/path" == "/exact/path"
 * - Extension match: ".py" matches "/script.py", "/cgi/script.py"
 * - Prefix match: "/admin" matches "/admin/users", "/admin/settings"
 *
 * The method returns the first matching handler found, respecting priority order.
 *
 * @param server_name Name of the server to search routes for
 * @param method HTTP method to match (GET, POST, etc.)
 * @param path URL path to match against registered routes
 * @return Pointer to handler function or nullptr if no matching route found
 */
const Router::Handler* Router::findHandler(const std::string& server_name, const std::string& method, const std::string& path) const {
    // std::cout << "DEBUG ROUTER: Looking for server: " << server_name << ", method: " << method << ", path: " << path << std::endl;

    // Step 1: Find the server in our routing table
    auto server_it = _routes.find(server_name);
    if (server_it == _routes.end()) {
        std::cout << "DEBUG ROUTER: Server not found in routing table" << std::endl;
        return nullptr; // Server not found in routing table
    }

    const auto& server_routes = server_it->second;
    std::cout << "DEBUG ROUTER: Server found, checking " << server_routes.size() << " routes" << std::endl;

    // Step 2: Try exact path match first (highest priority)
    auto path_it = server_routes.find(path);
    if (path_it != server_routes.end()) {
        std::cout << "DEBUG ROUTER: Exact path match found for: " << path << std::endl;
        auto method_it = path_it->second.find(method);
        if (method_it != path_it->second.end()) {
            std::cout << "DEBUG ROUTER: Handler found for method: " << method << std::endl;
            return &method_it->second; // Exact match found
        } else {
            std::cout << "DEBUG ROUTER: Method " << method << " not found for path " << path << std::endl;
        }
    } else {
        std::cout << "DEBUG ROUTER: No exact path match for: " << path << std::endl;
    }

    // Step 3: If no exact match, try advanced matching strategies
    for (const auto& route_pair : server_routes) {
        const std::string& route_path = route_pair.first;

        // Strategy A: Extension-based matching
        // Example: Route ".py" should match "/cgi-bin/script.py" or "/script.py"
        // Condition: route_path starts with '.', path is longer, ends with route_path
        if (!route_path.empty() && route_path[0] == '.' && path.length() > route_path.length()) {
            if (path.substr(path.length() - route_path.length()) == route_path) {
                auto method_it = route_pair.second.find(method);
                if (method_it != route_pair.second.end()) {
                    return &method_it->second; // Extension match found
                }
            }
        }
        // Strategy B: Prefix-based matching
        // Example: Route "/uploads" should match "/uploads/file.txt"
        // Condition: path starts with route_path, followed by '/' or end of string
        else if (path.length() > route_path.length() &&
                 path.substr(0, route_path.length()) == route_path &&
                 (route_path.back() == '/' || path[route_path.length()] == '/')) {
            auto method_it = route_pair.second.find(method);
            if (method_it != route_pair.second.end()) {
                return &method_it->second; // Prefix match found
            }
        }
    }

    return nullptr; // No matching route found
}


/**
 * @brief Find the matching location configuration for a given server and path
 *
 * This method searches through all location blocks in a server configuration
 * to find the most specific location that matches the requested path.
 *
 * Location Matching Logic:
 * 1. Exact Match: Direct path equality (highest priority)
 * 2. Prefix Match: Find the longest prefix match
 *
 * Examples:
 * - Path "/admin/users" with locations ["/", "/admin", "/admin/users"]
 *   -> Matches "/admin/users" (exact) or "/admin" (longest prefix)
 * - Path "/api/v1/users" with locations ["/", "/api"]
 *   -> Matches "/api" (longest prefix match)
 *
 * The method returns the most specific (longest) matching location.
 *
 * @param server The server configuration object to search in
 * @param path URL path to match against location patterns
 * @return Pointer to matching location or nullptr if no match found
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
 * @brief Generate HTML content for default error pages
 *
 * This method loads custom error page templates from the filesystem
 * based on HTTP status codes. If a specific error page file doesn't
 * exist or can't be read, it falls back to the generic 500 error page.
 *
 * Supported error pages:
 * - 400 Bad Request
 * - 404 Not Found
 * - 405 Method Not Allowed
 * - 413 Payload Too Large
 * - 500 Internal Server Error (fallback for all others)
 *
 * @param status HTTP status code (e.g., 404, 500)
 * @return HTML content string for the error page
 */
// Now using ErrorResponseBuilder::getErrorPageHtml instead

/**
 * @brief Set up a complete error response with appropriate status, headers, and body
 *
 * This utility function configures a Response object with all necessary components
 * for a proper HTTP error response:
 *
 * 1. Sets the appropriate HTTP status line (e.g., "404 Not Found")
 * 2. Sets Content-Type header to "text/html"
 * 3. Sets Content-Length header with the size of the error page
 * 4. Sets Connection header to "close" to terminate the connection
 * 5. Sets the response body with the appropriate HTML error page
 *
 * This function is used throughout the web server whenever an error condition
 * requires sending an error response to the client.
 *
 * @param res Response object to configure with error details
 * @param status HTTP status code for the error (e.g., 404, 500)
 */
// Now using ErrorResponseBuilder::setErrorResponse instead

/**
 * @brief Process an incoming HTTP request and route it to the appropriate handler
 *
 * This is the main entry point for request processing in the web server.
 * The method coordinates between routing and request processing by:
 * 1. Finding the appropriate handler for the request
 * 2. Delegating execution to the RequestProcessor
 * 3. Providing fallback mechanisms when no handler is found
 *
 * The Router focuses on routing logic while RequestProcessor handles
 * the complex request processing pipeline.
 *
 * @param server The server configuration for this request
 * @param req The incoming HTTP request object
 * @param res The response object to be populated by the handler
 */
void Router::handleRequest(const Server& server, const Request& req, Response& res) const {
    std::cout << "=== ROUTER HANDLE REQUEST START ===" << std::endl;

    // Extract method and path from the request
    std::string_view method_view = req.getMethod();
    std::string_view path_view = req.getPath();

    std::string method(method_view);
    std::string path(path_view);

    std::cout << "DEBUG ROUTER handleRequest: Server: " << server.getName() << ", Method: " << method << ", Path: " << path << std::endl;

    // Debug: Print the incoming request details
    // std::cout << "---------" << std::endl;
    req.print();
    // std::cout << "---------" << std::endl;

    // Find the appropriate handler for this request
    const Handler* handler = findHandler(server.getName(), method, path);

    // Find the matching location configuration for this request
    const Location* location = findLocation(server, path);
    std::cout << "DEBUG ROUTER: Found location: " << (location ? location->location : "nullptr") << std::endl;

    // Delegate to RequestProcessor for execution and fallback handling
    _requestProcessor.processRequest(server, req, handler, res, location);

    std::cout << "=== ROUTER HANDLE REQUEST END ===" << std::endl;
}



