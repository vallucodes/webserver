/**
 * @file RequestProcessor.cpp
 * @brief Implementation of the RequestProcessor class
 *
 * This file contains the implementation of the RequestProcessor class which
 * handles the complete HTTP request processing pipeline. It provides clean
 * separation of concerns and improved testability compared to embedding
 * all logic directly in the Router class.
 */

#include "RequestProcessor.hpp"
#include "handlers/Handlers.hpp"
#include <iostream>
#include <algorithm>

/**
 * @brief Default constructor for RequestProcessor
 */
RequestProcessor::RequestProcessor() {
    // Constructor can be used for initialization if needed
}

/**
 * @brief Destructor for RequestProcessor
 */
RequestProcessor::~RequestProcessor() {
    // Destructor can be used for cleanup if needed
}

/**
 * @brief Main request processing method
 */
void RequestProcessor::processRequest(const Server& server, const Request& req,
                                     const Handler* handler, Response& res, const Location* location) const {
    // Extract and validate request components
    std::string_view method_view = req.getMethod();
    std::string_view path_view = req.getPath();

    std::string method(method_view);
    std::string path(path_view);

    logRequestProcessing(req, "start", "Method: " + method + ", Path: " + path);

    // Validate and normalize the path
    if (!validatePath(path)) {
        logRequestProcessing(req, "validation_failed", "Invalid path: " + path);
        generateErrorResponse(res, http::BAD_REQUEST_400, "Invalid request path");
        return;
    }

    normalizePath(path);
    logRequestProcessing(req, "normalized", "Normalized path: " + path);

    // Execute handler if available
    std::cout << "DEBUG: Handler found: " << (handler ? "YES" : "NO") << ", Server: " << server.getName() << ", Path: " << path << std::endl;
    if (handler) {
        std::cout << "DEBUG: Executing handler for path: " << path << std::endl;
        if (executeHandlerSafely(handler, server, req, res, path, location)) {
            logRequestProcessing(req, "handler_executed", "Handler executed successfully");
            return;
        } else {
            std::cout << "DEBUG: Handler execution failed" << std::endl;
        }
    } else {
        std::cout << "DEBUG: No handler found, trying static file fallback" << std::endl;
    }

    // Fallback: try to serve as static file
    logRequestProcessing(req, "fallback", "Trying static file fallback");
    if (tryServeAsStaticFile(req, res, method)) {
        logRequestProcessing(req, "static_file_served", "Static file served successfully");
        return;
    }

    // All attempts failed - return 404
    logRequestProcessing(req, "not_found", "No handler or static file found");
    generateErrorResponse(res, http::NOT_FOUND_404, "Resource not found: " + path);
}

/**
 * @brief Validate request path for security issues
 */
bool RequestProcessor::validatePath(const std::string& path) const {
    // Check for null bytes (security issue)
    if (path.find('\0') != std::string::npos) {
        return false;
    }

    // Check for path traversal attempts
    if (path.find("../") != std::string::npos || path.find("..\\") != std::string::npos) {
        return false;
    }

    // Check for extremely long paths (potential DoS)
    if (path.length() > 2048) { // Reasonable limit for URLs
        return false;
    }

    // Check for invalid characters
    const std::string invalid_chars = "<>\"|?*";
    if (path.find_first_of(invalid_chars) != std::string::npos) {
        return false;
    }

    return true;
}

/**
 * @brief Normalize request path
 */
void RequestProcessor::normalizePath(std::string& path) const {
    // Remove query parameters
    size_t query_pos = path.find('?');
    if (query_pos != std::string::npos) {
        path = path.substr(0, query_pos);
    }

    // Remove URL fragments
    size_t fragment_pos = path.find('#');
    if (fragment_pos != std::string::npos) {
        path = path.substr(0, fragment_pos);
    }

    // Normalize multiple consecutive slashes
    size_t pos = 0;
    while ((pos = path.find("//", pos)) != std::string::npos) {
        path.replace(pos, 2, "/");
    }

    // Ensure path starts with /
    if (!path.empty() && path[0] != '/') {
        path = "/" + path;
    }

    // Normalize trailing slash for directory requests
    // Remove trailing slash unless it's the root path
    if (path.length() > 1 && path.back() == '/') {
        path.pop_back();
    }
}

/**
 * @brief Execute handler with proper error handling
 */
bool RequestProcessor::executeHandlerSafely(const Handler* handler, const Server& server __attribute__((unused)),
                                           const Request& req, Response& res,
                                           const std::string& path __attribute__((unused)), const Location* location) const {
    if (!handler) {
        return false;
    }

    try {
        // Execute the handler with the passed location
        (*handler)(req, res, location);
        return true;
    } catch (const std::exception& e) {
        logRequestProcessing(req, "handler_error", std::string("Exception: ") + e.what());
        generateErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, "Handler execution failed");
        return false;
    } catch (...) {
        logRequestProcessing(req, "handler_error", "Unknown exception occurred");
        generateErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, "Handler execution failed");
        return false;
    }
}

/**
 * @brief Try to serve request as static file
 */
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

/**
 * @brief Generate error response
 */
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
    res.setHeaders(http::CONNECTION, http::CONNECTION_CLOSE);

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

/**
 * @brief Log request processing information
 */
void RequestProcessor::logRequestProcessing(const Request& req, const std::string& stage,
                                           const std::string& details) const {
    std::cout << "[RequestProcessor] " << stage;
    if (!details.empty()) {
        std::cout << ": " << details;
    }
    std::cout << " (Path: " << req.getPath() << ")" << std::endl;
}
