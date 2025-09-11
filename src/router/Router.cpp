#include "../../inc/webserv.hpp"
#include "Router.hpp"
#include "handlers/Handlers.hpp"

// using namespace http;

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

void Router::setupRouter() {
    addRoute("GET", "/", get);
	addRoute("GET", "/index.html", get);
	// addRoute("GET", "/upload.html", submitFile);
	addRoute("GET", "/upload.html", get);
	addRoute("GET", "/upload_error.html", get);
	addRoute("GET", "/upload_success.html", get);

	addRoute("GET", "/imgs/lhaas.png", get);
	addRoute("GET", "/imgs/vlopatin.png", get);
	addRoute("GET", "/imgs/imunaev-.png", get);

	// Upload route - handles file uploads
	addRoute("POST", "/upload", post);


	// Debug: List all available routes
	listRoutes();
}

void Router::addRoute(std::string_view method, std::string_view path, Handler handler) {
    _routes[std::string(path)][std::string(method)] = std::move(handler);
}

const Router::Handler* Router::findHandler(const std::string& method, const std::string& path) const {
    auto path_it = _routes.find(path);
    if (path_it != _routes.end()) {
        auto method_it = path_it->second.find(method);
        if (method_it != path_it->second.end()) {
            return &method_it->second;
        }
    }
    return nullptr;
}

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

void setErrorResponse(Response& res, int status){
    if (status == http::NOT_FOUND_404) {
        res.setStatus(http::STATUS_NOT_FOUND_404);
    } else if (status == http::METHOD_NOT_ALLOWED_405) {
        res.setStatus(http::STATUS_METHOD_NOT_ALLOWED_405);
    } else if (status == http::INTERNAL_SERVER_ERROR_500) {
        res.setStatus(http::STATUS_INTERNAL_SERVER_ERROR_500);
    }
    res.setHeader("Content-Type", "text/html");
    res.setHeader("Content-Length", std::to_string(Router::getDefaultErrorPage(status).length()));
    res.setHeader("Connection", "close");
    res.setBody(Router::getDefaultErrorPage(status));
}

void Router::handleRequest(const Request& req, Response& res) const {
    std::string_view method_view = req.getMethod();
    std::string_view path_view = req.getPath();

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
    } else {
        // Path not found
        std::cout << "Path '" << path << "' not found in routes" << std::endl;
        setErrorResponse(res, http::NOT_FOUND_404);
    }
}



