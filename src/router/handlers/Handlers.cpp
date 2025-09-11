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

        // Check if it's multipart/form-data
        if (contentType.find("multipart/form-data") == std::string::npos) {
            std::string errorHtml = readFileToString(page::UPLOAD_ERROR_HTML);
            size_t pos = errorHtml.find("ERROR_MESSAGE_PLACEHOLDER");
            if (pos != std::string::npos) {
                errorHtml.replace(pos, 25, "Invalid content type. Expected multipart/form-data.");
            }
            setSuccessResponse(res, errorHtml, CONTENT_TYPE_HTML);
            return;
        }

        // Extract boundary from Content-Type header
        size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos == std::string::npos) {
            std::string errorHtml = readFileToString(page::UPLOAD_ERROR_HTML);
            size_t pos = errorHtml.find("ERROR_MESSAGE_PLACEHOLDER");
            if (pos != std::string::npos) {
                errorHtml.replace(pos, 25, "Invalid multipart boundary.");
            }
            setSuccessResponse(res, errorHtml, CONTENT_TYPE_HTML);
            return;
        }

        std::string boundary = "--" + contentType.substr(boundaryPos + 9);
        std::string bodyStr(body);

        // Find file content between boundaries
        size_t fileStart = bodyStr.find(boundary);
        if (fileStart == std::string::npos) {
            std::string errorHtml = readFileToString(page::UPLOAD_ERROR_HTML);
            size_t pos = errorHtml.find("ERROR_MESSAGE_PLACEHOLDER");
            if (pos != std::string::npos) {
                errorHtml.replace(pos, 25, "No file data found in request.");
            }
            setSuccessResponse(res, errorHtml, CONTENT_TYPE_HTML);
            return;
        }

        // Find the next boundary
        size_t fileEnd = bodyStr.find(boundary, fileStart + boundary.length());
        if (fileEnd == std::string::npos) {
            fileEnd = bodyStr.length();
        }

        // Extract file part
        std::string filePart = bodyStr.substr(fileStart, fileEnd - fileStart);

        // Parse filename from Content-Disposition header
        std::string filename;
        size_t filenamePos = filePart.find("filename=\"");
        if (filenamePos != std::string::npos) {
            size_t filenameEnd = filePart.find("\"", filenamePos + 10);
            if (filenameEnd != std::string::npos) {
                filename = filePart.substr(filenamePos + 10, filenameEnd - filenamePos - 10);
            }
        }

        if (filename.empty()) {
            std::string errorHtml = readFileToString(page::UPLOAD_ERROR_HTML);
            size_t pos = errorHtml.find("ERROR_MESSAGE_PLACEHOLDER");
            if (pos != std::string::npos) {
                errorHtml.replace(pos, 25, "No filename provided.");
            }
            setSuccessResponse(res, errorHtml, CONTENT_TYPE_HTML);
            return;
        }

        // Sanitize filename (remove path separators for security)
        filename.erase(std::remove_if(filename.begin(), filename.end(),
            [](char c) { return c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|'; }), filename.end());

        // Find actual file content (after headers)
        size_t contentStart = filePart.find("\r\n\r\n");
        if (contentStart == std::string::npos) {
            std::string errorHtml = readFileToString(page::UPLOAD_ERROR_HTML);
            size_t pos = errorHtml.find("ERROR_MESSAGE_PLACEHOLDER");
            if (pos != std::string::npos) {
                errorHtml.replace(pos, 25, "Invalid file format.");
            }
            setSuccessResponse(res, errorHtml, CONTENT_TYPE_HTML);
            return;
        }

        contentStart += 4; // Skip \r\n\r\n
        std::string fileContent = filePart.substr(contentStart);

        // Remove trailing \r\n if present
        if (fileContent.length() >= 2 && fileContent.substr(fileContent.length() - 2) == "\r\n") {
            fileContent = fileContent.substr(0, fileContent.length() - 2);
        }

        // Validate file size (max 1MB)
        if (fileContent.length() > 1024 * 1024) {
            std::string errorHtml = readFileToString(page::UPLOAD_ERROR_HTML);
            size_t pos = errorHtml.find("ERROR_MESSAGE_PLACEHOLDER");
            if (pos != std::string::npos) {
                errorHtml.replace(pos, 25, "File size exceeds 1MB limit.");
            }
            setSuccessResponse(res, errorHtml, CONTENT_TYPE_HTML);
            return;
        }

        // Save file to uploads directory
        std::string filePath = "www/uploads/" + filename;
        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile) {
            std::string errorHtml = readFileToString(page::UPLOAD_ERROR_HTML);
            size_t pos = errorHtml.find("ERROR_MESSAGE_PLACEHOLDER");
            if (pos != std::string::npos) {
                errorHtml.replace(pos, 25, "Failed to save file to server.");
            }
            setSuccessResponse(res, errorHtml, CONTENT_TYPE_HTML);
            return;
        }

        outFile.write(fileContent.c_str(), fileContent.length());
        outFile.close();

        // Success response
        std::string successHtml = readFileToString(page::UPLOAD_SUCCESS_HTML);

        // Replace placeholders with actual values
        std::string replacements[2][2] = {
            {"FILENAME_PLACEHOLDER", filename},
            {"FILESIZE_PLACEHOLDER", std::to_string(fileContent.length() / 1024.0).substr(0, 4)}
        };

        for (const auto& replacement : replacements) {
            size_t pos = successHtml.find(replacement[0]);
            while (pos != std::string::npos) {
                successHtml.replace(pos, replacement[0].length(), replacement[1]);
                pos = successHtml.find(replacement[0], pos + replacement[1].length());
            }
        }

        setSuccessResponse(res, successHtml, CONTENT_TYPE_HTML);

    } catch (const std::exception& e) {
        std::string errorHtml = readFileToString(page::UPLOAD_ERROR_HTML);
        size_t pos = errorHtml.find("ERROR_MESSAGE_PLACEHOLDER");
        if (pos != std::string::npos) {
            errorHtml.replace(pos, 25, "An unexpected error occurred during upload.");
        }
        setSuccessResponse(res, errorHtml, CONTENT_TYPE_HTML);
    }
}

// Handle DELETE requests for removing uploaded files
// @param req The incoming HTTP request with file path in URL
// @param res The response object to populate
void del(const Request& req, Response& res) {
    try {
        // Extract filename from URL path (e.g., /uploads/filename.txt -> filename.txt)
        std::string_view filePathView = req.getPath();

        // Validate path starts with /uploads/
        if (filePathView.length() < 9 || filePathView.substr(0, 9) != "/uploads/") {
            std::string errorHtml = readFileToString(page::UPLOAD_ERROR_HTML);
            size_t pos = errorHtml.find("ERROR_MESSAGE_PLACEHOLDER");
            if (pos != std::string::npos) {
                errorHtml.replace(pos, 25, "Invalid path. DELETE only allowed for /uploads/ directory.");
            }
            setSuccessResponse(res, errorHtml, CONTENT_TYPE_HTML);
            return;
        }

        // Extract filename from path
        std::string filename = std::string(filePathView.substr(9)); // Remove "/uploads/" prefix

        // Sanitize filename (remove path separators for security)
        filename.erase(std::remove_if(filename.begin(), filename.end(),
            [](char c) { return c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|'; }), filename.end());

        if (filename.empty()) {
            std::string errorHtml = readFileToString(page::UPLOAD_ERROR_HTML);
            size_t pos = errorHtml.find("ERROR_MESSAGE_PLACEHOLDER");
            if (pos != std::string::npos) {
                errorHtml.replace(pos, 25, "No filename provided in path.");
            }
            setSuccessResponse(res, errorHtml, CONTENT_TYPE_HTML);
            return;
        }

        // Construct full file path
        std::string filePath = page::WWW + "/uploads/" + filename;

        // Check if file exists before attempting deletion
        if (!std::filesystem::exists(filePath)) {
            std::string errorHtml = readFileToString(page::UPLOAD_ERROR_HTML);
            size_t pos = errorHtml.find("ERROR_MESSAGE_PLACEHOLDER");
            if (pos != std::string::npos) {
                errorHtml.replace(pos, 25, "File not found: " + filename);
            }
            setSuccessResponse(res, errorHtml, CONTENT_TYPE_HTML);
            return;
        }

        // Attempt to delete the file
        if (std::filesystem::remove(filePath)) {
            // Success response
            std::string successHtml = readFileToString(page::UPLOAD_SUCCESS_HTML);

            // Replace placeholders with deletion info
            std::string replacements[2][2] = {
                {"FILENAME_PLACEHOLDER", filename},
                {"FILESIZE_PLACEHOLDER", "deleted"}
            };

            for (const auto& replacement : replacements) {
                size_t pos = successHtml.find(replacement[0]);
                while (pos != std::string::npos) {
                    successHtml.replace(pos, replacement[0].length(), replacement[1]);
                    pos = successHtml.find(replacement[0], pos + replacement[1].length());
                }
            }

            // Also replace "Upload Successful" with "Deletion Successful"
            size_t uploadPos = successHtml.find("Upload Successful");
            if (uploadPos != std::string::npos) {
                successHtml.replace(uploadPos, 17, "Deletion Successful");
            }

            setSuccessResponse(res, successHtml, CONTENT_TYPE_HTML);
        } else {
            // Deletion failed
            std::string errorHtml = readFileToString(page::UPLOAD_ERROR_HTML);
            size_t pos = errorHtml.find("ERROR_MESSAGE_PLACEHOLDER");
            if (pos != std::string::npos) {
                errorHtml.replace(pos, 25, "Failed to delete file: " + filename);
            }
            setSuccessResponse(res, errorHtml, CONTENT_TYPE_HTML);
        }

    } catch (const std::filesystem::filesystem_error& e) {
        std::string errorHtml = readFileToString(page::UPLOAD_ERROR_HTML);
        size_t pos = errorHtml.find("ERROR_MESSAGE_PLACEHOLDER");
        if (pos != std::string::npos) {
            errorHtml.replace(pos, 25, "Filesystem error during deletion.");
        }
        setSuccessResponse(res, errorHtml, CONTENT_TYPE_HTML);
    } catch (const std::exception& e) {
        std::string errorHtml = readFileToString(page::UPLOAD_ERROR_HTML);
        size_t pos = errorHtml.find("ERROR_MESSAGE_PLACEHOLDER");
        if (pos != std::string::npos) {
            errorHtml.replace(pos, 25, "An unexpected error occurred during deletion.");
        }
        setSuccessResponse(res, errorHtml, CONTENT_TYPE_HTML);
    }
}


