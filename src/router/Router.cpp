#include "../../inc/webserv.hpp"
#include "Router.hpp"
#include "handlers/Handlers.hpp"
#include <algorithm>

Router::Router() {}

Router::~Router() {}

// Debug method to list all routes
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

// Initialize the router with routes based on server configurations
void Router::setupRouter(const std::vector<Server>& configs) {
    // Clear existing routes
    _routes.clear();

    // Iterate through each server configuration
    for (const auto& server : configs) {
        // Get server root directory for resolving relative paths
        std::string server_root = server.getRoot();

        // Iterate through each location in the server
        for (const auto& location : server.getLocations()) {
            std::string location_path = location.location;

            // Handle different location types:
            // 1. Root location "/"
            // 2. Exact paths like "/favicon.ico"
            // 3. Extension-based locations like ".py"
            // 4. Prefix paths like "/upload"

            // For each allowed method in this location, add a route
            for (const auto& method : location.allowed_methods) {
                // Choose the appropriate handler based on the location configuration
                Handler handler;

                // Determine handler based on location properties and HTTP method
                if (!location.cgi_path.empty() && !location.cgi_ext.empty()) {
                    // CGI location - handle CGI scripts for any method
                    handler = cgi;
                } else if (method == "POST" && !location.upload_path.empty()) {
                    // POST to upload location - handle file uploads
                    handler = post;
                } else if (method == "DELETE" && !location.upload_path.empty()) {
                    // DELETE from upload location - handle file deletions
                    handler = del;
                } else if (method == "GET" || method == "HEAD") {
                    // GET/HEAD requests - handle static file serving
                    handler = get;
                } else {
                    // Default handler for other methods
                    handler = get;
                }

                // Add the route
                addRoute(server.getName(), method, location_path, handler);

                // For GET requests, also add HEAD support (HTTP convention)
                // Only if HEAD is not already explicitly allowed
                if (method == "GET" && std::find(location.allowed_methods.begin(),
                    location.allowed_methods.end(), "HEAD") == location.allowed_methods.end()) {
                    addRoute(server.getName(), "HEAD", location_path, handler);
                }
            }
        }
    }

    // Debug: List all available routes
    listRoutes();
}

// Register a new route with specific HTTP method and path for a server
// @param server_name Name of the server this route belongs to
// @param method HTTP method (GET, POST, etc.)
// @param path URL path to match
// @param handler Function to handle requests for this route
void Router::addRoute(std::string_view server_name, std::string_view method, std::string_view path, Handler handler) {
    _routes[std::string(server_name)][std::string(path)][std::string(method)] = std::move(handler);
}

// Find the handler function for a given server, method and path
// @param server_name Name of the server to search routes for
// @param method HTTP method to match
// @param path URL path to match
// @return Pointer to handler function or nullptr if not found
const Router::Handler* Router::findHandler(const std::string& server_name, const std::string& method, const std::string& path) const {
    // First find the server in our routes
    auto server_it = _routes.find(server_name);
    if (server_it == _routes.end()) {
        return nullptr; // Server not found
    }

    const auto& server_routes = server_it->second;

    // First try exact match
    auto path_it = server_routes.find(path);
    if (path_it != server_routes.end()) {
        auto method_it = path_it->second.find(method);
        if (method_it != path_it->second.end()) {
            return &method_it->second;
        }
    }

    // If no exact match, try different location matching strategies
    for (const auto& route_pair : server_routes) {
        const std::string& route_path = route_pair.first;

        // Check for extension-based matching (e.g., ".py" matches "/cgi-bin/script.py")
        if (!route_path.empty() && route_path[0] == '.' && path.length() > route_path.length()) {
            if (path.substr(path.length() - route_path.length()) == route_path) {
                auto method_it = route_pair.second.find(method);
                if (method_it != route_pair.second.end()) {
                    return &method_it->second;
                }
            }
        }
        // Check for prefix matching (e.g., "/uploads" matches "/uploads/file.txt")
        else if (path.length() > route_path.length() &&
                 path.substr(0, route_path.length()) == route_path &&
                 (route_path.back() == '/' || path[route_path.length()] == '/')) {
            auto method_it = route_pair.second.find(method);
            if (method_it != route_pair.second.end()) {
                return &method_it->second;
            }
        }
    }

    return nullptr;
}

// Find the matching location for a given server and path
// @param server The server configuration
// @param path URL path to match
// @return Pointer to matching location or nullptr if not found
const Location* Router::findLocation(const Server& server, const std::string& path) const {
    const auto& locations = server.getLocations();
    const Location* best_match = nullptr;
    size_t best_match_length = 0;

    for (const auto& location : locations) {
        const std::string& location_path = location.location;

        // Exact match
        if (location_path == path) {
            return &location;
        }

        // Prefix match (e.g., "/admin" matches "/admin/page")
        if (path.length() > location_path.length() &&
            path.substr(0, location_path.length()) == location_path &&
            (location_path.back() == '/' || path[location_path.length()] == '/')) {
            if (location_path.length() > best_match_length) {
                best_match = &location;
                best_match_length = location_path.length();
            }
        }
    }

    return best_match;
}

// Default error page generator
// @param status HTTP status code
// @return HTML content for the error page
std::string Router::getDefaultErrorPage(int status) {
    switch (status) {
        case http::NOT_FOUND_404:
            return readFileToString(error_page::ERROR_PAGE_NOT_FOUND_404);
        case http::METHOD_NOT_ALLOWED_405:
            return readFileToString(error_page::ERROR_PAGE_METHOD_NOT_ALLOWED_405);
        case http::BAD_REQUEST_400:
            return readFileToString(error_page::ERROR_PAGE_BAD_REQUEST_400);
        case http::PAYLOAD_TOO_LARGE_413:
            return readFileToString(error_page::ERROR_PAGE_PAYLOAD_TOO_LARGE_413);
        case http::INTERNAL_SERVER_ERROR_500:
            return readFileToString(error_page::ERROR_PAGE_INTERNAL_SERVER_ERROR_500);
        default:
            return readFileToString(error_page::ERROR_PAGE_INTERNAL_SERVER_ERROR_500);
    }
}

// Set up a complete error response with appropriate status, headers, and body
// @param res Response object to configure
// @param status HTTP status code for the error
void setErrorResponse(Response& res, int status){
    if (status == http::NOT_FOUND_404) {
        res.setStatus(http::STATUS_NOT_FOUND_404);
    } else if (status == http::METHOD_NOT_ALLOWED_405) {
        res.setStatus(http::STATUS_METHOD_NOT_ALLOWED_405);
    } else if (status == http::BAD_REQUEST_400) {
        res.setStatus(http::STATUS_BAD_REQUEST_400);
    } else if (status == http::PAYLOAD_TOO_LARGE_413) {
        res.setStatus(http::STATUS_PAYLOAD_TOO_LARGE_413);
    } else if (status == http::INTERNAL_SERVER_ERROR_500) {
        res.setStatus(http::STATUS_INTERNAL_SERVER_ERROR_500);
    }
    res.setHeaders("Content-Type", "text/html");
    res.setHeaders("Content-Length", std::to_string(Router::getDefaultErrorPage(status).length()));
    res.setHeaders("Connection", "close");
    res.setBody(Router::getDefaultErrorPage(status));
}

// Process an incoming HTTP request and route it to appropriate handler
// @param server The server configuration for this request
// @param req The incoming HTTP request
// @param res The response object to populate
void Router::handleRequest(const Server& server, const Request& req, Response& res) const {
    std::string_view method_view = req.getMethod();
    std::string_view path_view = req.getPath();

    // std::cout << "---------" << std::endl;
    req.print();
    // std::cout << "---------" << std::endl;

    std::string method(method_view);
    std::string path(path_view);

    // Debug logging
    // std::cout << "Request: " << method << " " << path << std::endl;

    // Normalize path - remove query parameters and fragments
    size_t query_pos = path.find('?');
    if (query_pos != std::string::npos) {
        path = path.substr(0, query_pos);
        std::cout << "Normalized path (removed query): " << path << std::endl;
    }

    size_t fragment_pos = path.find('#');
    if (fragment_pos != std::string::npos) {
        path = path.substr(0, fragment_pos);
        std::cout << "Normalized path (removed fragment): " << path << std::endl;
    }

    if (const Handler* h = findHandler(server.getName(), method, path)) {
        try {
            const Location* location = findLocation(server, path);
            (*h)(req, res, location);
        } catch (...) {
            setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
        }
        return;
    }

    // If handler not found, check if path exists in routes
    auto path_it = _routes.find(path);
    if (path_it != _routes.end()) {
        // Path exists but method not allowed
        std::cout << "Path '" << path << "' exists but method '" << method << "' is not allowed" << std::endl;
        std::cout << "Allowed methods for '" << path << "': ";
        for (const auto& method_pair : path_it->second) {
            std::cout << method_pair.first << " ";
        }
        std::cout << std::endl;
        setErrorResponse(res, http::METHOD_NOT_ALLOWED_405);
        std::cout << "---------" << std::endl;
        res.print();
        std::cout << "---------" << std::endl;
    } else {
        // Path not found in routes - try to serve as static file
        std::cout << "Path '" << path << "' not found in routes, trying to serve as static file" << std::endl;

        // For GET requests, try to serve as static file
        if (method == "GET") {
            try {
                get(req, res, nullptr);
                std::cout << "---------" << std::endl;
                res.print();
                std::cout << "---------" << std::endl;
                return;
            } catch (...) {
                // Static file serving failed, return 404
                std::cout << "Static file serving failed for path '" << path << "'" << std::endl;
            }
        }

        // Path not found and not a static file
        std::cout << "Path '" << path << "' not found in routes and not a valid static file" << std::endl;
        setErrorResponse(res, http::NOT_FOUND_404);
        std::cout << "---------" << std::endl;
        res.print();
        std::cout << "---------" << std::endl;
    }
}



