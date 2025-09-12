#include "../../../inc/webserv.hpp"
#include "Handlers.hpp"
#include "../Router.hpp"

using namespace http;

#include <fstream>
#include <sstream>
#include <filesystem>
#include <cctype>

// Determine the MIME content type based on file extension
// @param filename The filename to analyze
// @return MIME type string (e.g., "text/html", "image/png")
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

// Helper constants for response formatting
const std::string CONNECTION_CLOSE = "close";
const std::string CONTENT_TYPE_HTML = "text/html";

// Helper functions for response formatting

// Set common HTTP headers for responses (Content-Type, Content-Length, Connection)
// @param res Response object to configure
// @param contentType MIME type for the response content
// @param contentLength Size of the response body in bytes
void setCommonHeaders(Response& res, const std::string& contentType, size_t contentLength) {
    res.setHeader("Content-Type", contentType);
    res.setHeader("Content-Length", std::to_string(contentLength));
    res.setHeader("Connection", CONNECTION_CLOSE);
}

// Configure a complete successful HTTP response with status, headers, and body
// @param res Response object to configure
// @param content The response body content
// @param contentType MIME type for the response content
void setSuccessResponse(Response& res, const std::string& content, const std::string& contentType) {
    res.setStatus(http::STATUS_OK_200);
    setCommonHeaders(res, contentType, content.length());
    res.setBody(content);
}

// Helper function to replace all occurrences of a placeholder in HTML template
// @param html The HTML template string
// @param placeholder The placeholder to replace
// @param replacement The replacement text
// @return Modified HTML string
std::string replacePlaceholder(std::string html, const std::string& placeholder, const std::string& replacement) {
    size_t pos = 0;
    while ((pos = html.find(placeholder, pos)) != std::string::npos) {
        html.replace(pos, placeholder.length(), replacement);
        pos += replacement.length();
    }
    return html;
}

// Create error HTML response from template
// @param errorMessage The error message to display
// @return Complete HTML error page
std::string createErrorHtml(const std::string& errorMessage) {
    std::string html = readFileToString(page::UPLOAD_ERROR_HTML);
    return replacePlaceholder(html, "ERROR_MESSAGE_PLACEHOLDER", errorMessage);
}

// Create success HTML response from template
// @param filename The filename that was processed
// @param filesize The file size information
// @return Complete HTML success page
std::string createSuccessHtml(const std::string& filename, const std::string& filesize) {
    std::string html = readFileToString(page::UPLOAD_SUCCESS_HTML);
    html = replacePlaceholder(html, "FILENAME_PLACEHOLDER", filename);
    html = replacePlaceholder(html, "FILESIZE_PLACEHOLDER", filesize);
    return html;
}

// Create deletion success HTML response
// @param filename The filename that was deleted
// @return Complete HTML deletion success page
std::string createDeletionSuccessHtml(const std::string& filename) {
    std::string html = createSuccessHtml(filename, "deleted");
    return replacePlaceholder(html, "Upload Successful", "Deletion Successful");
}

// Sanitize filename by removing path separators and other dangerous characters
// @param filename The filename to sanitize
// @return Sanitized filename
std::string sanitizeFilename(std::string filename) {
    const std::string forbiddenChars = "/\\:*?\"<>|";
    filename.erase(std::remove_if(filename.begin(), filename.end(),
        [&forbiddenChars](char c) { return forbiddenChars.find(c) != std::string::npos; }),
        filename.end());
    return filename;
}

// Handle GET requests for static files and pages
// @param req The incoming HTTP request
// @param res The response object to populate
void get(const Request& req, Response& res) {
    try {
        // Extract and validate file path
        std::string_view filePathView = req.getPath();
        if (filePathView.empty()) {
            setErrorResponse(res, http::NOT_FOUND_404);
            return;
        }

        std::string filePath;
        if (filePathView == page::ROOT_HTML || filePathView == page::INDEX_HTML_PATH) {
            filePath = page::INDEX_HTML;
        } else {
            filePath = page::WWW + std::string(filePathView);
        }

        // Attempt to read the requested file
        std::string fileContent = readFileToString(filePath);

        // Determine content type and send success response
        std::string contentType = getContentType(filePath);
        setSuccessResponse(res, fileContent, contentType);

    } catch (const std::runtime_error& e) {
        // File not found or read error
        setErrorResponse(res, http::NOT_FOUND_404);
    } catch (const std::exception& e) {
        // Unexpected error
        setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
    }
}

// Handle POST requests for file uploads
// @param req The incoming HTTP request containing multipart/form-data
// @param res The response object to populate
void post(const Request& req, Response& res) {
    try {
        std::string_view body = req.getBody();
        std::string contentType = std::string(req.getHeaders("Content-Type"));

        // Validate content type
        if (contentType.find("multipart/form-data") == std::string::npos) {
            setSuccessResponse(res, createErrorHtml("Invalid content type. Expected multipart/form-data."), CONTENT_TYPE_HTML);
            return;
        }

        // Extract boundary
        const size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos == std::string::npos) {
            setSuccessResponse(res, createErrorHtml("Invalid multipart boundary."), CONTENT_TYPE_HTML);
            return;
        }

        const std::string boundary = "--" + contentType.substr(boundaryPos + 9);
        const std::string bodyStr(body);

        // Find file boundaries
        const size_t fileStart = bodyStr.find(boundary);
        if (fileStart == std::string::npos) {
            setSuccessResponse(res, createErrorHtml("No file data found in request."), CONTENT_TYPE_HTML);
            return;
        }

        const size_t fileEnd = bodyStr.find(boundary, fileStart + boundary.length());
        const std::string filePart = bodyStr.substr(fileStart, fileEnd - fileStart);

        // Extract filename
        const size_t filenamePos = filePart.find("filename=\"");
        if (filenamePos == std::string::npos) {
            setSuccessResponse(res, createErrorHtml("No filename provided."), CONTENT_TYPE_HTML);
            return;
        }

        const size_t filenameEnd = filePart.find("\"", filenamePos + 10);
        if (filenameEnd == std::string::npos) {
            setSuccessResponse(res, createErrorHtml("Invalid filename format."), CONTENT_TYPE_HTML);
            return;
        }

        std::string filename = sanitizeFilename(filePart.substr(filenamePos + 10, filenameEnd - filenamePos - 10));
        if (filename.empty()) {
            setSuccessResponse(res, createErrorHtml("No filename provided."), CONTENT_TYPE_HTML);
            return;
        }

        // Extract file content
        const size_t contentStart = filePart.find("\r\n\r\n");
        if (contentStart == std::string::npos) {
            setSuccessResponse(res, createErrorHtml("Invalid file format."), CONTENT_TYPE_HTML);
            return;
        }

        std::string fileContent = filePart.substr(contentStart + 4);

        // Remove trailing \r\n if present
        if (fileContent.length() >= 2 && fileContent.substr(fileContent.length() - 2) == "\r\n") {
            fileContent = fileContent.substr(0, fileContent.length() - 2);
        }

        // Validate file size
        if (fileContent.length() > 1024 * 1024) {
            setSuccessResponse(res, createErrorHtml("File size exceeds 1MB limit."), CONTENT_TYPE_HTML);
            return;
        }

        // Save file
        const std::string filePath = "www/upload/" + filename;
        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile) {
            setSuccessResponse(res, createErrorHtml("Failed to save file to server."), CONTENT_TYPE_HTML);
            return;
        }

        outFile.write(fileContent.c_str(), fileContent.length());
        outFile.close();

        // Success response
        const std::string fileSizeStr = std::to_string(fileContent.length() / 1024.0).substr(0, 4) + " KB";
        setSuccessResponse(res, createSuccessHtml(filename, fileSizeStr), CONTENT_TYPE_HTML);

    } catch (const std::exception&) {
        setSuccessResponse(res, createErrorHtml("An unexpected error occurred during upload."), CONTENT_TYPE_HTML);
    }
}

// Handle DELETE requests for removing uploaded files
// @param req The incoming HTTP request with file path in URL
// @param res The response object to populate
void del(const Request& req, Response& res) {
    try {
        const std::string_view filePathView = req.getPath();

        // Validate path
        if (filePathView.length() < 9 || filePathView.substr(0, 9) != "/uploads/") {
            setSuccessResponse(res, createErrorHtml("Invalid path. DELETE only allowed for /uploads/ directory."), CONTENT_TYPE_HTML);
            return;
        }

        std::string filename = sanitizeFilename(std::string(filePathView.substr(9)));
        if (filename.empty()) {
            setSuccessResponse(res, createErrorHtml("No filename provided in path."), CONTENT_TYPE_HTML);
            return;
        }

        const std::string filePath = page::WWW + "/uploads/" + filename;

        // Check if file exists
        if (!std::filesystem::exists(filePath)) {
            setSuccessResponse(res, createErrorHtml("File not found: " + filename), CONTENT_TYPE_HTML);
            return;
        }

        // Attempt deletion
        if (std::filesystem::remove(filePath)) {
            setSuccessResponse(res, createDeletionSuccessHtml(filename), CONTENT_TYPE_HTML);
        } else {
            setSuccessResponse(res, createErrorHtml("Failed to delete file: " + filename), CONTENT_TYPE_HTML);
        }

    } catch (const std::filesystem::filesystem_error&) {
        setSuccessResponse(res, createErrorHtml("Filesystem error during deletion."), CONTENT_TYPE_HTML);
    } catch (const std::exception&) {
        setSuccessResponse(res, createErrorHtml("An unexpected error occurred during deletion."), CONTENT_TYPE_HTML);
    }
}


