#include "../../inc/webserv.hpp"
#include "Router.hpp"
#include "handlers/Handlers.hpp"

Router::Router() {}

Router::~Router() {}

// Debug method to list all routes
void Router::listRoutes() const {
    std::cout << "Available routes:" << std::endl;
    for (const auto& path_pair : _routes) {
        std::cout << "  " << path_pair.first << " -> ";
        for (const auto& method_pair : path_pair.second) {
            std::cout << method_pair.first << " ";
        }
        std::cout << std::endl;
    }
}

// Initialize the router with default routes for static files and upload endpoints
void Router::setupRouter() {
    addRoute("GET", "/", get);
	addRoute("GET", "/index.html", get);
	addRoute("GET", "/upload.html", get);
	addRoute("GET", "/upload_error.html", get);
	addRoute("GET", "/upload_success.html", get);

	addRoute("GET", "/imgs/lhaas.png", get);
	addRoute("GET", "/imgs/vlopatin.png", get);
	addRoute("GET", "/imgs/imunaev-.png", get);

	// Upload route - handles file uploads
	addRoute("POST", "/uploads", post);
	addRoute("GET", "/uploads", get);

	addRoute("GET", "/delete.html", get);
	addRoute("DELETE", "/uploads", del);

	// CGI routes - handle CGI scripts based on file extensions
	// Since no parser exists yet, we manually add routes for CGI files
	// These routes will be handled by the CGI handler which checks file extensions
	addRoute("GET", "/cgi-bin/script.py", cgi);
	addRoute("POST", "/cgi-bin/script.py", cgi);
	addRoute("GET", "/cgi-bin/hello.c", cgi);
	addRoute("POST", "/cgi-bin/hello.c", cgi);
	addRoute("GET", "/cgi-bin/hello.ts", cgi);
	addRoute("POST", "/cgi-bin/hello.ts", cgi);
	addRoute("GET", "/cgi-bin/hello.js", cgi);
	addRoute("POST", "/cgi-bin/hello.js", cgi);

	// Generic CGI route for any file with CGI extension in www directory
	// This will catch any CGI files not explicitly listed above
	// The CGI handler will check the file extension and handle accordingly


	// Debug: List all available routes
	listRoutes();
}

// Register a new route with specific HTTP method and path
// @param method HTTP method (GET, POST, etc.)
// @param path URL path to match
// @param handler Function to handle requests for this route
void Router::addRoute(std::string_view method, std::string_view path, Handler handler) {
    _routes[std::string(path)][std::string(method)] = std::move(handler);
}

// Find the handler function for a given method and path
// @param method HTTP method to match
// @param path URL path to match
// @return Pointer to handler function or nullptr if not found
const Router::Handler* Router::findHandler(const std::string& method, const std::string& path) const {
    // First try exact match
    auto path_it = _routes.find(path);
    if (path_it != _routes.end()) {
        auto method_it = path_it->second.find(method);
        if (method_it != path_it->second.end()) {
            return &method_it->second;
        }
    }

    // If no exact match, try prefix matching for uploads routes
    for (const auto& route_pair : _routes) {
        const std::string& route_path = route_pair.first;
        // Check if the route path is a prefix of the requested path
        if (path.length() > route_path.length() &&
            path.substr(0, route_path.length()) == route_path &&
            (route_path == "/uploads" || route_path == "/uploads/")) {
            auto method_it = route_pair.second.find(method);
            if (method_it != route_pair.second.end()) {
                return &method_it->second;
            }
        }
    }

    return nullptr;
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
    } else if (status == http::INTERNAL_SERVER_ERROR_500) {
        res.setStatus(http::STATUS_INTERNAL_SERVER_ERROR_500);
    }
    res.setHeaders("Content-Type", "text/html");
    res.setHeaders("Content-Length", std::to_string(Router::getDefaultErrorPage(status).length()));
    res.setHeaders("Connection", "close");
    res.setBody(Router::getDefaultErrorPage(status));
}

// Process an incoming HTTP request and route it to appropriate handler
// @param req The incoming HTTP request
// @param res The response object to populate
void Router::handleRequest(const Request& req, Response& res) const {
    std::string_view method_view = req.getMethod();
    std::string_view path_view = req.getPath();

    std::cout << "---------" << std::endl;
    req.print();
    std::cout << "---------" << std::endl;

    std::string method(method_view);
    std::string path(path_view);

    // Debug logging
    std::cout << "Request: " << method << " " << path << std::endl;

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

    if (const Handler* h = findHandler(method, path)) {
        try {
            (*h)(req, res);
        } catch (...) {
            setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
        }
        return;
    }

    // If handler not found, check if path exists
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
        // Path not found
        std::cout << "Path '" << path << "' not found in routes" << std::endl;
        setErrorResponse(res, http::NOT_FOUND_404);
        std::cout << "---------" << std::endl;
        res.print();
        std::cout << "---------" << std::endl;
    }
}



