#include "Router.hpp"

// Constructor
Router::Router() {}

// Destructor
Router::~Router() {}

// Register a new route mapping
void Router::addRoute(const std::string& method,
                      const std::string& path,
                      Handler handler) {
    _routes[path][method] = std::move(handler);
}

void Router::get(const std::string& path, Handler handler) {
    addRoute("GET", path, std::move(handler));
}

void Router::post(const std::string& path, Handler handler) {
    addRoute("POST", path, std::move(handler));
}

void Router::del(const std::string& path, Handler handler) {
    addRoute("DELETE", path, std::move(handler));
}

// Internal method to find a handler for a specific method and path
const Router::Handler* Router::findHandler(std::string_view method, std::string_view path) const {
    // First level lookup: find the path in the routes map
    auto path_it = _routes.find(std::string(path));
    if (path_it != _routes.end()) {
        // Path found - now look for the method in the inner map
        auto method_it = path_it->second.find(std::string(method));
        if (method_it != path_it->second.end()) {
            // Both path and method found - return pointer to handler
            return &method_it->second;
        }
    }
    // Either path or method not found
    return nullptr;
}

// Main request handling method
void Router::handleRequest(const Request& req, Response& res) const {
    // Extract the HTTP method and path from the request
    std::string_view method = req.getMethod(); // e.g. "GET"
    std::string_view path   = req.getPath();   // e.g. "/users/123"

    // Try to find a handler
    if (const Handler* h = findHandler(method, path)) {
        // Handler found, process the request with exception safety
        try {
            (*h)(req, res);
        } catch (...) {
            res.setStatus("500 Internal Server Error");
            res.setBody(getDefaultErrorPage(500));
        }
        return;
    }

    // No handler found - distinguish between 404 and 405
    auto path_it = _routes.find(std::string(path));
    if (path_it != _routes.end()) {
        // Path exists, but method not allowed
        res.setStatus("405 Method Not Allowed");
        res.setBody(getDefaultErrorPage(405));
    } else {
        // Path not found
        res.setStatus("404 Not Found");
        res.setBody(getDefaultErrorPage(404));
    }
}

// Default error page generator (fixed with added 405 case)
std::string Router::getDefaultErrorPage(int status) {
    switch (status) {
        case 404:
            return "<html><body><h1>404 Not Found</h1><p>The requested resource was not found.</p></body></html>";
        case 405:
            return "<html><body><h1>405 Method Not Allowed</h1><p>Invalid method for this resource.</p></body></html>";
        case 500:
            return "<html><body><h1>500 Internal Server Error</h1><p>Something went wrong on the server.</p></body></html>";
        default:
            return "<html><body><h1>Error</h1><p>An unexpected error occurred.</p></body></html>";
    }
}
