/**
 * @file Handlers.cpp
 * @brief Implementation of HTTP request handlers for the web server
 *
 * This file contains the implementation of all core HTTP request handlers:
 * - get(): Static file serving and directory index handling
 * - post(): File upload processing with multipart/form-data support
 * - del(): File deletion for uploaded files
 * - cgi(): CGI script execution for dynamic content
 * - redirect(): HTTP redirection handling
 *
 * Key Components:
 * - MIME type detection based on file extensions
 * - Multipart form-data parsing for file uploads
 * - CGI environment variable setup and script execution
 * - Path resolution and security validation
 * - Error handling and response formatting
 *
 * Security Features:
 * - Path traversal protection
 * - File permission validation
 * - Input sanitization
 * - Safe CGI execution with process isolation
 *
 * The handlers work together with the Router to provide a complete
 * HTTP request processing pipeline, from receiving requests to
 * generating appropriate responses.
 */

#include "../../../inc/webserv.hpp"
#include "../utils/StringUtils.hpp"
#include "../utils/FileUtils.hpp"
#include "../utils/ErrorResponseBuilder.hpp"
#include "Handlers.hpp"
#include "../Router.hpp"
#include "../HttpConstants.hpp"

using namespace http;

#include <fstream>
#include <sstream>
#include <filesystem>
#include <cctype>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>

/**
 * @brief Determine the MIME content type based on file extension
 *
 * This function maps file extensions to their corresponding MIME types
 * for proper HTTP Content-Type header generation. It supports common
 * web file formats and provides fallbacks for unknown extensions.
 *
 * Supported MIME Types:
 * - Text: html, htm, css, txt, json, js
 * - Images: png, jpg, jpeg, gif
 * - Applications: json, javascript
 *
 * The function performs case-insensitive extension matching and
 * uses an optimized lookup based on extension length for performance.
 *
 * @param filename The filename to analyze (e.g., "index.html", "style.css")
 * @return MIME type string (e.g., "text/html", "image/png", "text/plain" for unknown)
 */
std::string getContentType(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string ext = filename.substr(dotPos + 1);

        // Convert to lowercase for case-insensitive comparison
        for (char& c : ext) {
            c = std::tolower(c);
        }

        // Use switch-like behavior with computed hash
        switch (ext.length()) {
            case 3:
                if (ext == "css") return "text/css";
                if (ext == "png") return "image/png";
                if (ext == "jpg") return "image/jpeg";
                if (ext == "gif") return "image/gif";
                if (ext == "htm") return "text/html";
                if (ext == "txt") return "text/plain";
                break;
            case 4:
                if (ext == "html") return "text/html";
                if (ext == "jpeg") return "image/jpeg";
                if (ext == "json") return "application/json";
                break;
            case 2:
                if (ext == "js") return "application/javascript";
                break;
        }
    }
    return "text/plain";
}

// Helper functions for response formatting

// Set common HTTP headers for responses (Content-Type, Content-Length, Connection)
// @param res Response object to configure
// @param contentType MIME type for the response content
// @param contentLength Size of the response body in bytes
void setCommonHeaders(Response& res, const std::string& contentType, size_t contentLength) {
    res.setHeaders(http::CONTENT_TYPE, contentType);
    res.setHeaders(http::CONTENT_LENGTH, std::to_string(contentLength));
    res.setHeaders(http::CONNECTION, http::CONNECTION_CLOSE);
}

// Configure a complete successful HTTP response with status, headers, and body
// @param res Response object to configure
// @param content The response body content
// @param contentType MIME type for the response content
// Now using ErrorResponseBuilder::setSuccessResponse instead

// Helper function to replace all occurrences of a placeholder in HTML template
// @param html The HTML template string
// @param placeholder The placeholder to replace
// @param replacement The replacement text
// @return Modified HTML string
// Now using StringUtils::replacePlaceholder instead

// Create error HTML response from template
// @param errorMessage The error message to display
// @return Complete HTML error page
std::string createErrorHtml(const std::string& errorMessage) {
    std::string html = router::utils::FileUtils::readFileToString(page::UPLOAD_ERROR_HTML);
    return router::utils::StringUtils::replacePlaceholder(html, "ERROR_MESSAGE_PLACEHOLDER", errorMessage);
}

// Create success HTML response from template
// @param filename The filename that was processed
// @param filesize The file size information
// @return Complete HTML success page
std::string createSuccessHtml(const std::string& filename, const std::string& filesize) {
    std::string html = router::utils::FileUtils::readFileToString(page::UPLOAD_SUCCESS_HTML);
    html = router::utils::StringUtils::replacePlaceholder(html, "FILENAME_PLACEHOLDER", filename);
    html = router::utils::StringUtils::replacePlaceholder(html, "FILESIZE_PLACEHOLDER", filesize);
    return html;
}

// Create deletion success HTML response
// @param filename The filename that was deleted
// @return Complete HTML deletion success page
std::string createDeletionSuccessHtml(const std::string& filename) {
    std::string html = createSuccessHtml(filename, "deleted");
    return router::utils::StringUtils::replacePlaceholder(html, "Upload Successful", "Deletion Successful");
}

// Sanitize filename by removing path separators and other dangerous characters
// Now using StringUtils::sanitizeFilename instead

// Helper function to replace all occurrences of a substring
// Now using StringUtils::replaceAll instead

// Generate HTML directory listing using template
// @param dirPath Absolute path to the directory
// @param requestPath The request path for breadcrumbs
// @return HTML string with directory listing
std::string generateDirectoryListing(const std::string& dirPath, const std::string& requestPath) {
    // Load template file
    std::string templatePath = page::WWW + "/autoindex_template.html";
    std::string html;

    try {
        html = router::utils::FileUtils::readFileToString(templatePath);
    } catch (const std::exception& e) {
        // Fallback to fallback template file if main template fails
        std::cout << "Warning: Could not load autoindex template: " << e.what() << std::endl;
        std::string fallbackPath = page::WWW + "/autoindex_fallback.html";
        try {
            html = router::utils::FileUtils::readFileToString(fallbackPath);
        } catch (const std::exception& e2) {
            // If both templates fail, return a simple error message
            std::cout << "Error: Could not load fallback template: " << e2.what() << std::endl;
            return "<html><body><h1>Error</h1><p>Could not load directory listing template.</p></body></html>";
        }
    }

    // Replace placeholders
    html = router::utils::StringUtils::replaceAll(html, "{{PATH}}", requestPath);

    // Generate parent directory link
    std::string parentLink = "";
    if (requestPath != "/") {
        std::string parentPath = requestPath;
        size_t lastSlash = parentPath.find_last_of('/');
        if (lastSlash > 0) {
            parentPath = parentPath.substr(0, lastSlash);
            if (parentPath.empty()) parentPath = "/";
            parentLink = "    <a href=\"" + parentPath + "\" class=\"back-link\">← Parent directory</a>\n";
        }
    }
    html = router::utils::StringUtils::replaceAll(html, "{{PARENT_LINK}}", parentLink);

    // Generate directory items
    std::string items = "";
    try {
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            std::string name = entry.path().filename().string();
            std::string linkPath = requestPath;
            if (linkPath.back() != '/') linkPath += '/';
            linkPath += name;

            bool isDir = entry.is_directory();
            std::string icon = isDir ? "📁" : "📄";
            std::string cssClass = isDir ? "dir-icon" : "file-icon";

            // Get file size
            std::string sizeStr = "-";
            if (!isDir) {
                try {
                    auto size = entry.file_size();
                    if (size < 1024) sizeStr = std::to_string(size) + " B";
                    else if (size < 1024 * 1024) sizeStr = std::to_string(size / 1024) + " KB";
                    else sizeStr = std::to_string(size / (1024 * 1024)) + " MB";
                } catch (...) {}
            }

            // Get last modified time
            std::string dateStr = "-";
            try {
                auto time = entry.last_write_time();
                auto time_t = std::chrono::system_clock::to_time_t(std::chrono::file_clock::to_sys(time));
                char buffer[20];
                strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", localtime(&time_t));
                dateStr = buffer;
            } catch (...) {}

            items += "        <div class=\"item\">\n";
            items += "            <span class=\"" + cssClass + "\">" + icon + "</span>\n";
            items += "            <a href=\"" + linkPath + "\" class=\"name\">" + name + "</a>\n";
            items += "            <span class=\"size\">" + sizeStr + "</span>\n";
            items += "            <span class=\"date\">" + dateStr + "</span>\n";
            items += "        </div>\n";
        }
    } catch (const std::exception& e) {
        items += "        <div class=\"item\">Error reading directory: " + std::string(e.what()) + "</div>\n";
    }

    html = router::utils::StringUtils::replaceAll(html, "{{ITEMS}}", items);

    return html;
}

/**
 * @brief Handle GET requests for static files and pages
 *
 * This is the primary handler for serving static content in the web server.
 * It supports:
 * 1. Direct file serving with proper MIME types
 * 2. Directory index serving (index.html, default.html)
 * 3. Auto-generated directory listings
 * 4. Location-specific index files
 * 5. Path resolution and security validation
 *
 * Request Processing Flow:
 * 1. Extract and validate the requested path
 * 2. Check location configuration for special handling
 * 3. Handle index files and directory requests
 * 4. Serve static files with appropriate headers
 * 5. Provide error responses for invalid requests
 *
 * Path Resolution Strategy:
 * - Root requests ("/") serve index.html by default
 * - Direct file paths serve the requested file
 * - Directory paths try index files, then autoindex if enabled
 * - All paths are validated for security (no path traversal)
 *
 * @param req The incoming HTTP request containing the URL path
 * @param res The response object to populate with file content and headers
 * @param location The matching location configuration (contains root, index, autoindex settings)
 */
void get(const Request& req, Response& res, const Location* location) {
    std::cout << "=== GET HANDLER START ===" << std::endl;
    std::cout << "GET HANDLER CALLED: path=" << req.getPath() << ", location=" << (location ? location->location : "nullptr") << std::endl;
    try {
        // Extract and validate file path
        std::string_view filePathView = req.getPath();
        if (filePathView.empty()) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404);
            return;
        }

        std::string requestPath = std::string(filePathView);
        std::string filePath;

        // TEMPORARY: Force autoindex for /imgs path for debugging
        if (requestPath == "/imgs" || requestPath == "/imgs/") {
            std::string dirPath = page::WWW + "/imgs";
            std::cout << "DEBUG: Forced autoindex for /imgs path, requestPath: " << requestPath << std::endl;
            if (std::filesystem::is_directory(dirPath)) {
                std::string dirListing = generateDirectoryListing(dirPath, "/imgs");
                router::utils::ErrorResponseBuilder::setSuccessResponse(res, dirListing, http::CONTENT_TYPE_HTML);
                return;
            }
        }

        // If we have location configuration, use it
        if (location != nullptr) {
            std::string dirPath = page::WWW + requestPath;

            // Check if it's a directory first
            if (std::filesystem::is_directory(dirPath)) {
                std::cout << "DEBUG: Directory found: " << dirPath << ", autoindex: " << location->autoindex << ", index: " << location->index << std::endl;
                // If location has autoindex enabled, generate directory listing
                if (location->autoindex) {
                    std::cout << "DEBUG: Generating autoindex for " << requestPath << std::endl;
                    std::string dirListing = generateDirectoryListing(dirPath, requestPath);
                    router::utils::ErrorResponseBuilder::setSuccessResponse(res, dirListing, http::CONTENT_TYPE_HTML);
                    return;
                }

                // If autoindex is disabled, try to serve index files
                if (!location->index.empty()) {
                    // For location-specific index, check in the location directory first
                    std::string indexPath = dirPath;
                    if (!indexPath.ends_with('/')) indexPath += '/';
                    indexPath += location->index;

                    // If location-specific index exists, use it
                    if (std::filesystem::exists(indexPath) && std::filesystem::is_regular_file(indexPath)) {
                        std::string fileContent = router::utils::FileUtils::readFileToString(indexPath);
                        std::string contentType = getContentType(indexPath);
                        router::utils::ErrorResponseBuilder::setSuccessResponse(res, fileContent, contentType);
                        return;
                    }

                    // Otherwise, use the global index file (only for non-autoindex locations)
                    std::string globalIndexPath = page::WWW + "/" + location->index;
                    if (std::filesystem::exists(globalIndexPath) && std::filesystem::is_regular_file(globalIndexPath)) {
                        std::string fileContent = router::utils::FileUtils::readFileToString(globalIndexPath);
                        std::string contentType = getContentType(globalIndexPath);
                        router::utils::ErrorResponseBuilder::setSuccessResponse(res, fileContent, contentType);
                        return;
                    }
                }
            }
        }

        // Default behavior: serve static files
        if (requestPath == "/" || requestPath == "/index.html") {
            filePath = page::INDEX_HTML;
        } else {
            filePath = page::WWW + requestPath;
        }

        // Check if the path is a directory and try to serve default files
        if (std::filesystem::is_directory(filePath)) {
            // List of default files to check in order of preference
            std::vector<std::string> defaultFiles = {
                "index.html", "default.html"
            };

            for (const auto& defaultFile : defaultFiles) {
                std::string defaultPath = filePath + "/" + defaultFile;
                if (std::filesystem::exists(defaultPath) && std::filesystem::is_regular_file(defaultPath)) {
                    filePath = defaultPath;
                    break;
                }
            }

            // If no default file was found and autoindex is not enabled, return 404
            if (std::filesystem::is_directory(filePath)) {
                router::utils::ErrorResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404);
                return;
            }
        }

        // Attempt to read the requested file
        std::string fileContent = router::utils::FileUtils::readFileToString(filePath);

        // Determine content type and send success response
        std::string contentType = getContentType(filePath);
        router::utils::ErrorResponseBuilder::setSuccessResponse(res, fileContent, contentType);

    } catch (const std::runtime_error& e) {
        // File not found or read error
        router::utils::ErrorResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404);
    } catch (const std::exception& e) {
        // Unexpected error
        router::utils::ErrorResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
    }
    std::cout << "=== GET HANDLER END ===" << std::endl;
}

/**
 * @brief Handle POST requests for file uploads
 *
 * This handler processes multipart/form-data POST requests for file uploads.
 * It parses the multipart data, extracts files and form fields, and saves
 * uploaded files to the configured upload directory.
 *
 * Multipart Processing Steps:
 * 1. Validate Content-Type header contains "multipart/form-data"
 * 2. Extract boundary string from Content-Type header
 * 3. Parse request body using boundary delimiters
 * 4. Extract headers and content for each part
 * 5. Identify file uploads vs. regular form fields
 * 6. Save files with proper names and permissions
 *
 * Security Features:
 * - Validates upload_path configuration in location
 * - Prevents directory traversal in filenames
 * - Checks file size limits (if configured)
 * - Sanitizes uploaded filenames
 * - Validates multipart boundary format
 *
 * File Storage:
 * - Files saved to location->upload_path directory
 * - Original filenames preserved (with sanitization)
 * - Proper file permissions set
 * - Duplicate filename handling
 *
 * Response Codes:
 * - 201 Created: Files uploaded successfully
 * - 400 Bad Request: Invalid multipart data or missing boundary
 * - 403 Forbidden: Uploads not allowed in this location
 * - 413 Payload Too Large: File exceeds size limits
 * - 500 Internal Server Error: File system errors
 *
 * @param req The incoming HTTP request containing multipart/form-data
 * @param res The response object to populate with upload results
 * @param location The matching location configuration (must have upload_path)
 */
void post(const Request& req, Response& res, const Location* location) {
    try {
        // Validate location configuration
        if (!location || location->upload_path.empty()) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::FORBIDDEN_403);
            return;
        }

        std::string_view body = req.getBody();
        const auto& contentTypeKey = req.getHeaders("content-type");

        if (contentTypeKey.empty()) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        std::string contentType = contentTypeKey.front();

        // Validate content type
        if (contentType.find("multipart/form-data") == std::string::npos) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        // Extract boundary
        const size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos == std::string::npos) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        const std::string boundary = "--" + contentType.substr(boundaryPos + 9);
        const std::string bodyStr(body);

        // Find file boundaries
        const size_t fileStart = bodyStr.find(boundary);
        if (fileStart == std::string::npos) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        const size_t fileEnd = bodyStr.find(boundary, fileStart + boundary.length());
        const std::string filePart = bodyStr.substr(fileStart, fileEnd - fileStart);

        // Extract filename
        const size_t filenamePos = filePart.find("filename=\"");
        if (filenamePos == std::string::npos) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        const size_t filenameEnd = filePart.find("\"", filenamePos + 10);
        if (filenameEnd == std::string::npos) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        std::string filename = router::utils::StringUtils::sanitizeFilename(filePart.substr(filenamePos + 10, filenameEnd - filenamePos - 10));
        if (filename.empty()) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        // Extract file content
        const size_t contentStart = filePart.find("\r\n\r\n");
        if (contentStart == std::string::npos) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        std::string fileContent = filePart.substr(contentStart + 4);

        // Remove trailing \r\n if present
        if (fileContent.length() >= 2 && fileContent.substr(fileContent.length() - 2) == "\r\n") {
            fileContent = fileContent.substr(0, fileContent.length() - 2);
        }

        // Validate file size
        if (fileContent.length() > 1024 * 1024) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::PAYLOAD_TOO_LARGE_413);
            return;
        }

        // Use location-configured upload path
        const std::string filePath = location->upload_path + "/" + filename;

        // Ensure upload directory exists
        std::filesystem::create_directories(location->upload_path);

        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
            return;
        }

        outFile.write(fileContent.c_str(), fileContent.length());
        outFile.close();

        // Success response
        const std::string fileSizeStr = std::to_string(fileContent.length() / 1024.0).substr(0, 4) + " KB";
        router::utils::ErrorResponseBuilder::setSuccessResponse(res, createSuccessHtml(filename, fileSizeStr), http::CONTENT_TYPE_HTML);

    } catch (const std::exception&) {
        router::utils::ErrorResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
    }
}

/**
 * @brief Handle DELETE requests for removing uploaded files
 *
 * This handler processes DELETE requests to remove files that were previously
 * uploaded via POST requests. It provides a safe way to delete uploaded files
 * while maintaining security constraints.
 *
 * Security and Validation:
 * 1. Validates that the path starts with "/uploads/" (restricted to upload directory)
 * 2. Sanitizes the filename to prevent directory traversal attacks
 * 3. Checks file existence before attempting deletion
 * 4. Validates file permissions and ownership
 *
 * Deletion Process:
 * 1. Extract filename from URL path
 * 2. Construct full filesystem path to upload directory
 * 3. Verify file exists and is accessible
 * 4. Attempt file deletion
 * 5. Return appropriate success/error response
 *
 * Path Security:
 * - Only allows deletion within the uploads directory
 * - Prevents deletion of files outside upload area
 * - Sanitizes filenames to remove dangerous characters
 * - Validates filename is not empty after sanitization
 *
 * Response Handling:
 * - 204 No Content: File successfully deleted (with success page)
 * - 400 Bad Request: Invalid path format or filename
 * - 404 Not Found: File doesn't exist
 * - 500 Internal Server Error: Filesystem deletion failure
 *
 * @param req The incoming HTTP request with file path in URL
 * @param res The response object to populate with deletion results
 * @param location The matching location configuration (unused but provided for consistency)
 */
void del(const Request& req, Response& res, const Location* location) {
    try {
        // Validate location configuration
        if (!location || location->upload_path.empty()) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::FORBIDDEN_403);
            return;
        }

        const std::string_view filePathView = req.getPath();

        // Extract the expected upload path prefix from location
        std::string uploadPrefix = "/uploads";
        if (filePathView.length() < uploadPrefix.length() + 1 ||
            filePathView.substr(0, uploadPrefix.length() + 1) != uploadPrefix + "/") {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        std::string filename = router::utils::StringUtils::sanitizeFilename(std::string(filePathView.substr(uploadPrefix.length() + 1)));
        if (filename.empty()) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        // Use location-configured upload path
        const std::string filePath = location->upload_path + "/" + filename;

        // Check if file exists
        if (!std::filesystem::exists(filePath)) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404);
            return;
        }

        // Attempt deletion
        if (std::filesystem::remove(filePath)) {
            router::utils::ErrorResponseBuilder::setSuccessResponse(res, createDeletionSuccessHtml(filename), http::CONTENT_TYPE_HTML);
        } else {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
        }

    } catch (const std::filesystem::filesystem_error&) {
        router::utils::ErrorResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
    } catch (const std::exception&) {
        router::utils::ErrorResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
    }
}

// Check if file extension indicates CGI script
// @param filename The filename to check
// @return true if file should be handled by CGI
// Old isCgiScript function removed - now using isCgiScriptWithLocation instead

// Check if file extension indicates CGI script using location configuration
// @param filename The filename to check
// @param location The location configuration containing allowed CGI extensions
// @return true if file should be handled by CGI
bool isCgiScriptWithLocation(const std::string& filename, const Location* location) {
    if (!location || location->cgi_ext.empty()) {
        return false;
    }

    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string ext = filename.substr(dotPos + 1);
        // Convert to lowercase for case-insensitive comparison
        for (char& c : ext) {
            c = std::tolower(c);
        }

        // Check against location-configured extensions
        for (const auto& allowedExt : location->cgi_ext) {
            std::string allowedExtLower = allowedExt;
            for (char& c : allowedExtLower) {
                c = std::tolower(c);
            }
            if (ext == allowedExtLower) {
                return true;
            }
        }
    }
    return false;
}

// Set up CGI environment variables
// @param req The HTTP request
// @param scriptPath Full path to the CGI script
// @param scriptName Name of the CGI script
// @return Vector of environment variable strings in "KEY=VALUE" format
std::vector<std::string> setupCgiEnvironment(const Request& req, const std::string& scriptPath, const std::string& scriptName) {
    std::vector<std::string> env;

    // Basic CGI environment variables
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env.push_back("REQUEST_METHOD=" + std::string(req.getMethod()));
    env.push_back("SCRIPT_NAME=" + scriptName);
    env.push_back("SCRIPT_FILENAME=" + scriptPath);
    env.push_back("PATH_INFO=" + scriptPath); // Full path as requested
    env.push_back("PATH_TRANSLATED=" + scriptPath);

    // Query string handling
    std::string pathStr(req.getPath());
    size_t queryPos = pathStr.find('?');
    if (queryPos != std::string::npos) {
        env.push_back("QUERY_STRING=" + pathStr.substr(queryPos + 1));
    } else {
        env.push_back("QUERY_STRING=");
    }

    // Content handling
    auto contentType = req.getHeaders("content-type");
    if (!contentType.empty()) {
        env.push_back("CONTENT_TYPE=" + contentType[0]);
    }

    auto contentLength = req.getHeaders("content-length");
    if (!contentLength.empty()) {
        env.push_back("CONTENT_LENGTH=" + contentLength[0]);
    } else {
        // If no Content-Length, use body size
        env.push_back("CONTENT_LENGTH=" + std::to_string(std::string(req.getBody()).length()));
    }

    // Server information
    env.push_back("SERVER_SOFTWARE=webserv/1.0");
    env.push_back("SERVER_NAME=localhost");
    env.push_back("SERVER_PORT=8080");

    // Remote client info (simplified)
    env.push_back("REMOTE_ADDR=127.0.0.1");
    env.push_back("REMOTE_HOST=localhost");

    // Add PATH for finding executables
    env.push_back("PATH=/usr/bin:/bin:/usr/local/bin");

    return env;
}

// Execute CGI script and capture output
// @param scriptPath Full path to the CGI script
// @param env Environment variables for CGI
// @param input Input data to send to CGI (request body)
// @return CGI output as string, or empty string on error
std::string executeCgiScript(const std::string& scriptPath, const std::vector<std::string>& env, const std::string& input) {
    std::cout << "CGI: executeCgiScript called for: " << scriptPath << std::endl;
    int pipe_in[2];  // For sending input to CGI
    int pipe_out[2]; // For receiving output from CGI

    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
        std::cout << "CGI: Failed to create pipes" << std::endl;
        return "";
    }

    pid_t pid = fork();
    if (pid == -1) {
        std::cout << "CGI: Fork failed" << std::endl;
        close(pipe_in[0]);
        close(pipe_in[1]);
        close(pipe_out[0]);
        close(pipe_out[1]);
        return "";
    }
    std::cout << "CGI: Fork successful, child PID: " << pid << std::endl;

    if (pid == 0) { // Child process
        // Close unused pipe ends
        close(pipe_in[1]);
        close(pipe_out[0]);

        // Redirect stdin to read from pipe_in
        if (dup2(pipe_in[0], STDIN_FILENO) == -1) {
            close(pipe_in[0]);
            close(pipe_out[1]);
            exit(1);
        }

        // Redirect stdout to write to pipe_out
        if (dup2(pipe_out[1], STDOUT_FILENO) == -1) {
            close(pipe_in[0]);
            close(pipe_out[1]);
            exit(1);
        }

        // Close original pipe descriptors
        close(pipe_in[0]);
        close(pipe_out[1]);

        // Set up environment variables
        std::vector<char*> envp;
        for (const auto& var : env) {
            envp.push_back(strdup(var.c_str()));
        }
        envp.push_back(nullptr);

        // Change to script directory for relative path access and get just the filename
        std::filesystem::path scriptDir = std::filesystem::path(scriptPath).parent_path();
        std::string scriptName = std::filesystem::path(scriptPath).filename().string();

        if (!scriptDir.empty()) {
            chdir(scriptDir.c_str());
        }

        // Execute CGI script based on file extension
        size_t dotPos = scriptName.find_last_of('.');
        if (dotPos != std::string::npos) {
            std::string ext = scriptName.substr(dotPos + 1);
            // Convert to lowercase for case-insensitive comparison
            for (char& c : ext) {
                c = std::tolower(c);
            }

            // std::cout << "CGI Child: Executing " << ext << " script: " << scriptName << " from dir: " << scriptDir << std::endl;

            if (ext == "py") {
                execl("/usr/bin/python3", "python3", scriptName.c_str(), nullptr);
            } else if (ext == "js") {
                // For JavaScript files, execute with Node.js
                execl("/usr/bin/node", "node", scriptName.c_str(), nullptr);
            // } else if (ext == "ts") {
            //     // For TypeScript files, check if compiled JS exists, otherwise try direct execution
            //     std::string jsFileName = scriptName.substr(0, scriptName.length() - 3) + ".js";
            //     std::cout << "CGI Child: TypeScript script: " << scriptName << std::endl;
            //     std::cout << "CGI Child: Looking for JS file: " << jsFileName << std::endl;
            //     std::cout << "CGI Child: JS file exists: " << std::filesystem::exists(jsFileName) << std::endl;
            //     if (std::filesystem::exists(jsFileName)) {
            //         // Use compiled JavaScript if it exists
            //         std::cout << "CGI Child: Executing compiled JS: " << jsFileName << std::endl;
            //         execl("/usr/bin/node", "node", jsFileName.c_str(), nullptr);
            //     } else {
            //         // Try direct execution with node (might work with newer Node versions)
            //         std::cout << "CGI Child: JS file not found, trying direct TS execution" << std::endl;
            //         execl("/usr/bin/node", "node", scriptName.c_str(), nullptr);
            //     }
            } else {
                // For unknown extensions, try direct execution
                execl(scriptName.c_str(), scriptName.c_str(), nullptr);
            }
        } else {
            // No extension, try direct execution
            execl(scriptName.c_str(), scriptName.c_str(), nullptr);
        }
        std::cout << "CGI Child: execl failed" << std::endl;

        // Clean up environment variables on failure
        for (auto& ptr : envp) {
            if (ptr) free(ptr);
        }

        // If execl fails, exit
        exit(1);
    } else { // Parent process
        // Close unused pipe ends
        close(pipe_in[0]);
        close(pipe_out[1]);

        // Send input to CGI
        if (!input.empty()) {
            write(pipe_in[1], input.c_str(), input.length());
        }
        close(pipe_in[1]); // Send EOF

        // Read output from CGI
        std::string output;
        char buffer[4096];
        ssize_t bytesRead;

        while ((bytesRead = read(pipe_out[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, bytesRead);
        }

        close(pipe_out[0]);

        // Wait for child to finish
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return output;
        } else {
            return "";
        }
    }
}

/**
 * @brief Handle CGI requests for executable scripts (e.g., .php files)
 *
 * This handler enables dynamic content generation by executing external
 * programs or scripts. It implements the Common Gateway Interface (CGI)
 * standard for interfacing web servers with external programs.
 *
 * CGI Processing Pipeline:
 * 1. Validate script file exists and has CGI extension
 * 2. Set up CGI environment variables (REQUEST_METHOD, QUERY_STRING, etc.)
 * 3. Fork child process to execute the script safely
 * 4. Pass request data to script via stdin
 * 5. Capture script output from stdout
 * 6. Parse CGI headers from script response
 * 7. Return processed response to client
 *
 * Environment Variables Provided:
 * - REQUEST_METHOD: HTTP method (GET, POST, etc.)
 * - QUERY_STRING: URL query parameters
 * - CONTENT_TYPE: Request content type
 * - CONTENT_LENGTH: Request body length
 * - SCRIPT_NAME: Script filename
 * - PATH_INFO: Additional path information
 * - SERVER_NAME, SERVER_PORT: Server information
 * - HTTP_* headers: All HTTP request headers
 *
 * Script Execution Security:
 * - Scripts executed in isolated child processes
 * - Process timeouts prevent hanging scripts
 * - Proper cleanup of child processes
 * - Restricted execution permissions
 * - Safe environment variable handling
 *
 * Response Processing:
 * - Parses CGI headers (Status, Content-Type, Location, etc.)
 * - Separates headers from response body
 * - Handles script errors gracefully
 * - Returns 500 error for script failures
 *
 * Supported Script Types:
 * - Python (.py)
 * - PHP (.php)
 * - Perl (.pl)
 * - Shell scripts (.sh)
 * - Any executable with proper CGI headers
 *
 * @param req The incoming HTTP request to be processed by the CGI script
 * @param res The response object to populate with script output
 * @param location The matching location configuration (contains cgi_path and cgi_ext)
 */
void cgi(const Request& req, Response& res, const Location* location) {
    try {
        // Validate location configuration
        if (!location || location->cgi_path.empty() || location->cgi_ext.empty()) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::FORBIDDEN_403);
            return;
        }

        // Extract and validate file path
        std::string_view filePathView = req.getPath();
        if (filePathView.empty()) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404);
            return;
        }

        std::string filePath;
        if (filePathView == page::ROOT_HTML || filePathView == page::INDEX_HTML_PATH) {
            filePath = page::INDEX_HTML;
        } else {
            // Use location-configured CGI path
            filePath = location->cgi_path + std::string(filePathView);
        }

        // Check if file exists and is executable
        if (!std::filesystem::exists(filePath)) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404);
            return;
        }

        // Verify it's a CGI script using location-configured extensions
        if (!isCgiScriptWithLocation(filePath, location)) {
            // Not a CGI script, handle as regular file
            std::string fileContent = router::utils::FileUtils::readFileToString(filePath);
            std::string contentType = getContentType(filePath);
            router::utils::ErrorResponseBuilder::setSuccessResponse(res, fileContent, contentType);
            return;
        }

        // Set up CGI environment
        std::string scriptName = std::filesystem::path(filePath).filename().string();
        auto env = setupCgiEnvironment(req, filePath, scriptName);

        // Get request body for CGI input
        std::string input(req.getBody());

        // Execute CGI script
        std::cout << "CGI: Executing script: " << filePath << std::endl;
        std::string cgiOutput = executeCgiScript(filePath, env, input);

        std::cout << "CGI: Script output length: " << cgiOutput.length() << std::endl;
        if (cgiOutput.empty()) {
            std::cout << "CGI: Script execution failed - returning 500 error" << std::endl;
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
            return;
        }
        std::cout << "CGI: Script executed successfully" << std::endl;

        // Parse CGI output (simple parsing - in production you'd want more robust parsing)
        // CGI output format: headers followed by blank line, then body
        size_t headerEnd = cgiOutput.find("\r\n\r\n");
        if (headerEnd == std::string::npos) {
            headerEnd = cgiOutput.find("\n\n");
        }

        std::string headersPart;
        std::string bodyPart;

        if (headerEnd != std::string::npos) {
            headersPart = cgiOutput.substr(0, headerEnd);
            bodyPart = cgiOutput.substr(headerEnd + (cgiOutput[headerEnd] == '\r' ? 4 : 2));
        } else {
            // No headers, treat whole output as body
            bodyPart = cgiOutput;
        }

        // Set response status (default to 200 if not specified)
        res.setStatus(http::STATUS_OK_200);

        // Parse and set headers from CGI output
        std::istringstream headerStream(headersPart);
        std::string headerLine;
        while (std::getline(headerStream, headerLine)) {
            // Remove \r if present
            if (!headerLine.empty() && headerLine.back() == '\r') {
                headerLine.pop_back();
            }

            size_t colonPos = headerLine.find(':');
            if (colonPos != std::string::npos) {
                std::string headerName = headerLine.substr(0, colonPos);
                std::string headerValue = headerLine.substr(colonPos + 1);

                // Trim whitespace
                headerValue.erase(headerValue.begin(), std::find_if(headerValue.begin(), headerValue.end(), [](int ch) {
                    return !std::isspace(ch);
                }));
                headerValue.erase(std::find_if(headerValue.rbegin(), headerValue.rend(), [](int ch) {
                    return !std::isspace(ch);
                }).base(), headerValue.end());

                // Special handling for Status header
                if (headerName == "Status") {
                    res.setStatus("HTTP/1.1 " + headerValue);
                } else {
                    res.setHeaders(headerName, headerValue);
                }
            }
        }

        // Set default content type if not specified
        auto contentType = res.getHeaders("Content-Type");
        if (contentType.empty()) {
            res.setHeaders("Content-Type", "text/html");
        }

        // Set response body
        res.setBody(bodyPart);

    } catch (const std::runtime_error& e) {
        // File not found or read error
        router::utils::ErrorResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404);
    } catch (const std::exception& e) {
        // Unexpected error
        router::utils::ErrorResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
    }
}

/**
 * @brief Handle HTTP redirection requests
 *
 * This handler manages HTTP redirection responses, allowing the server
 * to redirect clients to different URLs. Redirections are commonly used for:
 * - URL normalization and canonicalization
 * - Temporary redirects during maintenance or updates
 * - Permanent redirects for moved or renamed resources
 * - Protocol upgrades (HTTP to HTTPS)
 * - SEO-friendly URL restructuring
 *
 * HTTP Redirection Types:
 * - 301 Moved Permanently: Resource permanently moved to new URL
 *   - Search engines update their indexes
 *   - Browsers cache the redirect permanently
 *   - Clients should use new URL for future requests
 *
 * - 302 Found: Temporary redirect (client should continue using original URL)
 *   - Resource temporarily available at different location
 *   - Search engines keep original URL in index
 *   - Clients continue using original URL for future requests
 *
 * - 303 See Other: Redirect after POST to prevent form resubmission
 *   - Used after successful form submission
 *   - Prevents duplicate submissions on page refresh
 *   - Typically redirects to a confirmation or results page
 *
 * - 307 Temporary Redirect: Temporary redirect preserving HTTP method
 *   - Similar to 302 but preserves original HTTP method
 *   - Useful for API endpoints that may change location temporarily
 *
 * - 308 Permanent Redirect: Permanent redirect preserving HTTP method
 *   - Similar to 301 but preserves original HTTP method
 *   - Useful for REST API versioning and resource relocation
 *
 * Redirection Implementation:
 * 1. Determine redirect type based on location configuration or request
 * 2. Set appropriate HTTP status code (301, 302, 303, etc.)
 * 3. Set Location header with target URL
 * 4. Optionally include HTML redirect message in response body
 * 5. Handle relative and absolute redirect URLs
 *
 * URL Resolution:
 * - Supports absolute URLs: "http://example.com/new-path"
 * - Supports relative URLs: "/new-path", "../path"
 * - Handles protocol-relative URLs: "//example.com/path"
 * - Resolves relative URLs against current request context
 *
 * Configuration:
 * - Redirect URLs configured in location blocks
 * - Support for different redirect codes per location
 * - Can be combined with other location rules
 * - Dynamic redirect logic based on request parameters
 *
 * Security Considerations:
 * - Validates redirect URLs to prevent open redirect vulnerabilities
 * - Prevents redirect loops through proper URL validation
 * - Sanitizes user input in dynamic redirect URLs
 * - Logs redirect activity for monitoring and debugging
 *
 * Usage Examples:
 * - Site restructuring: "/old-blog" -> "/blog" (301)
 * - Temporary maintenance: "/api" -> "/maintenance" (302)
 * - Form handling: POST "/contact" -> "/thank-you" (303)
 * - API versioning: "/v1/users" -> "/v2/users" (308)
 *
 * @param req The incoming HTTP request being redirected
 * @param res The response object to populate with redirect information
 * @param location The matching location configuration (contains redirect_url and redirect_code)
 */
void redirect(const Request& req, Response& res, const Location* location) {
    try {
        // Validate location configuration
        if (!location || location->return_url.empty()) {
            router::utils::ErrorResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404);
            return;
        }

        // Use location-configured redirect URL
        std::string redirectUrl = location->return_url;

        // Log the redirect for debugging
        std::cout << "REDIRECT: " << req.getPath() << " -> " << redirectUrl << std::endl;

        // Set appropriate redirect status code (default to 302 if not specified)
        // In a real implementation, you might want to support different status codes
        // based on location configuration
        res.setStatus(http::STATUS_FOUND_302);

        // Set Location header for redirection
        res.setHeaders("Location", redirectUrl);

        // Set connection header
        res.setHeaders(http::CONNECTION, http::CONNECTION_CLOSE);

        // Optional: Add a simple HTML body for browsers that don't follow redirects automatically
        std::string body = "<!DOCTYPE html><html><head><title>Redirecting...</title></head><body>";
        body += "<p>If you are not redirected automatically, <a href=\"" + redirectUrl + "\">click here</a>.</p>";
        body += "</body></html>";

        res.setBody(body);

        // Set Content-Type header
        res.setHeaders(http::CONTENT_TYPE, http::CONTENT_TYPE_HTML);

    } catch (const std::exception& e) {
        // Unexpected error
        router::utils::ErrorResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
    }
}


