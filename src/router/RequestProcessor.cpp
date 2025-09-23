/**
 * @file RequestProcessor.cpp
 * @brief Implementation of RequestProcessor class
 */

#include "RequestProcessor.hpp"
#include "utils/HttpResponseBuilder.hpp"
#include "handlers/Handlers.hpp"
#include <iostream> // for std::cout, std::endl
#include <algorithm> // for std::find

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
                                        Response& res, const Location* location) const {
    // Extract and validate request components
    std::string_view method_view = req.getMethod();
    std::string_view path_view = req.getPath();

    std::string method(method_view);
    std::string path(path_view);

    // Validate HTTP method - return 400 Bad Request for unsupported methods
    if (method != http::GET && method != http::POST && method != http::DELETE) {
        router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
        return;
    }
    /*
    // Validate and normalize the path
    if (!validatePath(path)) {
        router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
        return;
    }

    normalizePath(path);
    */

    // Execute handler if available
    if (handler) {
        if (executeHandler(handler, req, res, location)) {
            return;
        }
    }

    // Fallback: try to serve as static file
    if (tryServeAsStaticFile(req, res, method)) {
        return;
    }

    // All attempts failed - return 404
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404);
}

/** Validate request path */
/*
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
*/

/** Normalize request path */
/*
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
*/
/** Execute handler */
bool RequestProcessor::executeHandler(const Handler* handler,
                                           const Request& req, Response& res,
                                           const Location* location) const {
    if (!handler) {
        return false;
    }
    try {
        // Execute the handler with the passed location
        (*handler)(req, res, location);
        return true;
    } catch (const std::exception& e) {
        router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
        return false;
    } catch (...) {
        router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
        return false;
    }
}

/** Serve static file */
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

/** Generate error response */
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
    // Default to keep-alive for HTTP/1.1 compatibility
    res.setHeaders(http::CONNECTION, http::CONNECTION_KEEP_ALIVE);

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
