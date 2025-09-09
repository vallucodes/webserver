#include "../../../inc/webserv.hpp"
#include "Handlers.hpp"
#include "../Router.hpp"

using namespace http;

#include <fstream>
#include <sstream>
#include <filesystem>
#include <cctype>

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
void setCommonHeaders(Response& res, const std::string& contentType, size_t contentLength) {
    res.setHeader("Content-Type", contentType);
    res.setHeader("Content-Length", std::to_string(contentLength));
    res.setHeader("Connection", CONNECTION_CLOSE);
}

void setSuccessResponse(Response& res, const std::string& content, const std::string& contentType) {
    res.setStatus(http::STATUS_OK_200);
    setCommonHeaders(res, contentType, content.length());
    res.setBody(content);
}

// Main page handler - serves requested file or returns appropriate error
void getMainPageHandler(const Request& req, Response& res) {
    try {
        // Extract and validate file path
        std::string_view filePathView = req.getPath();
        if (filePathView.empty()) {
            setErrorResponse(res, http::NOT_FOUND_404);
            return;
        }

        std::string filePath(filePathView);

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

