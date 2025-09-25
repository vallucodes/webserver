/**
 * @file Handlers.cpp
 * @brief Implementation of HTTP request handlers
 */

#include "Handlers.hpp"
#include "../utils/StringUtils.hpp"
#include "../utils/FileUtils.hpp"
#include "../utils/HttpResponseBuilder.hpp"
#include "../utils/ValidationUtils.hpp"
#include "../handlers/CgiExecutor.hpp"
#include "../utils/Utils.hpp"
#include "../HttpConstants.hpp"
#include "../../server/Server.hpp"

#include <fstream> // for std::ifstream, std::ofstream
#include <filesystem> // for std::filesystem::directory_iterator, std::filesystem::path, std::filesystem::exists, std::filesystem::is_directory, std::filesystem::is_regular_file, std::filesystem::create_directories, std::filesystem::remove, std::filesystem::file_size, std::filesystem::last_write_time
#include <cctype> // for std::tolower, std::isspace
#include <unistd.h> // for pipe, fork, dup2, close, write, read, chdir, execve, STDIN_FILENO, STDOUT_FILENO
#include <ctime> // for time, time_t, strftime
#include <sys/wait.h> // for waitpid, WNOHANG, WIFEXITED, WEXITSTATUS
#include <algorithm> // for std::transform, std::find, std::remove_if
#include <sstream> // for std::istringstream

using namespace http;
// #include <signal.h> // for kill, SIGKILL
// #include <cstdlib> // for std::stoul
// #include <fcntl.h> // for fcntl, F_GETFL, F_SETFL, O_NONBLOCK


// ***************** HELPERS ***************** //



/** Create simple success message */
std::string createSuccessMessage(const std::string& filename, const std::string& action) {
    return "File '" + filename + "' " + action + " successfully!";
}

/** Create simple error message */
std::string createErrorMessage(const std::string& errorMessage) {
    return "Error: " + errorMessage;
}

/** Set common HTTP headers */
void setCommonHeaders(Response& res, const std::string& contentType, size_t contentLength, const Request& req) {
    res.setHeaders(http::CONTENT_TYPE, contentType);
    res.setHeaders(http::CONTENT_LENGTH, std::to_string(contentLength));

    // Set connection header based on keep-alive logic
    if (router::utils::shouldKeepAlive(req)) {
        res.setHeaders(http::CONNECTION, http::CONNECTION_KEEP_ALIVE);
    } else {
        res.setHeaders(http::CONNECTION, http::CONNECTION_CLOSE);
    }
}

/** Generate HTML directory listing */
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

        // Remove trailing slash if present
        if (parentPath.back() == '/') {
            parentPath.pop_back();
        }

        // Find the last slash to get the parent directory
        size_t lastSlash = parentPath.find_last_of('/');
        if (lastSlash != std::string::npos) {
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

bool handleDirectoryRequest(const std::string& dirPath, const std::string& requestPath,
                           const Location* location, Response& res, const Request& req) {
    // Try autoindex first if enabled
    if (location && location->autoindex) {
        std::string dirListing = generateDirectoryListing(dirPath, requestPath);
        // router::utils::HttpResponseBuilder::setSuccessResponse(res, dirListing, http::CONTENT_TYPE_HTML, req);
        router::utils::HttpResponseBuilder::setSuccessResponse(res, dirListing, http::CONTENT_TYPE_HTML);
        return true;
    }

    // Combine location-specific, global, and default index files
    std::vector<std::string> indexPaths;
    if (location && !location->index.empty()) {
        std::string indexPath = dirPath;
        if (!indexPath.ends_with('/')) indexPath += '/';
        indexPaths.push_back(indexPath + location->index); // Location-specific index
        indexPaths.push_back(page::WWW + "/" + location->index); // Global index
    }
    for (const auto& defaultFile : page::DEFAULT_INDEX_FILES) {
        indexPaths.push_back(dirPath + "/" + defaultFile); // Default index files
    }

    // Try each index file in order
    for (const auto& path : indexPaths) {
        if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
            std::string fileContent = router::utils::FileUtils::readFileToString(path);
            std::string contentType = router::utils::FileUtils::getContentType(path);
            router::utils::HttpResponseBuilder::setSuccessResponse(res, fileContent, contentType, req);
            return true;
        }
    }

    return false;
}


/**
 * @brief Serve a static file
 * @param filePath Path to the file to serve
 * @param res Response object
 * @param req HTTP request
 * @return true if served successfully
 */
bool serveStaticFile(const std::string& filePath, Response& res, const Request& req) {
  try {
    std::string fileContent = router::utils::FileUtils::readFileToString(filePath);
    std::string contentType = router::utils::FileUtils::getContentType(filePath);
    router::utils::HttpResponseBuilder::setSuccessResponse(res, fileContent, contentType, req);
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

// ***************** GET HANDLER ***************** //

/**
 * @brief Handle GET requests for static files
 * @param req HTTP request
 * @param res Response object
 * @param location Location configuration
 */
void get(const Request& req, Response& res, const Location* location) {
  try {
    // Extract and validate file path
    std::string_view filePathView = req.getPath();
    if (filePathView.empty()) {
      router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
      return;
    }

    std::string requestPath = std::string(filePathView);
    std::string filePath = router::utils::StringUtils::determineFilePathBasic(requestPath);

    // Handle directory requests
    if (std::filesystem::is_directory(filePath)) {
      if (handleDirectoryRequest(filePath, requestPath, location, res, req)) {
        return;
      }
      // No index file found and autoindex disabled
      router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
      return;
    }

    // Serve static file
    if (serveStaticFile(filePath, res, req)) {
      return;
    }

    // File not found
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);

  } catch (const std::runtime_error&) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
  } catch (const std::exception&) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
    // router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
  }
}

// ***************** POST HANDLER ***************** //

/** Handle POST requests for file uploads */
void post(const Request& req, Response& res, const Location* location, const std::string& server_root) {
    try {
        // Validate location configuration
        if (!location || location->upload_path.empty()) {
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::FORBIDDEN_403, req);
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::FORBIDDEN_403);
            return;
        }

        // Handle chunked request body if needed
        std::string processedBody = std::string(req.getBody());

        if (router::utils::isChunked(req)) {
            processedBody = router::utils::parseChunkedRequestBody(processedBody);
        }

        const auto& contentTypeKey = req.getHeaders("content-type");

        if (contentTypeKey.empty()) {
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
            return;
        }

        std::string contentType = contentTypeKey.front();

        // Validate content type
        if (contentType.find("multipart/form-data") == std::string::npos) {
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
            return;
        }

        // Extract boundary
        const size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos == std::string::npos) {
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        const std::string boundary = "--" + contentType.substr(boundaryPos + 9);
        const std::string bodyStr(processedBody);

        // Find file boundaries
        const size_t fileStart = bodyStr.find(boundary);
        if (fileStart == std::string::npos) {
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
            return;
        }

        const size_t fileEnd = bodyStr.find(boundary, fileStart + boundary.length());
        const std::string filePart = bodyStr.substr(fileStart, fileEnd - fileStart);

        // Extract filename
        const size_t filenamePos = filePart.find("filename=\"");
        if (filenamePos == std::string::npos) {
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        const size_t filenameEnd = filePart.find("\"", filenamePos + 10);
        if (filenameEnd == std::string::npos) {
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
            return;
        }

        std::string filename = filePart.substr(filenamePos + 10, filenameEnd - filenamePos - 10);
        if (filename.empty()) {
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
            return;
        }

        // Extract file content
        const size_t contentStart = filePart.find("\r\n\r\n");
        if (contentStart == std::string::npos) {
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
            return;
        }

        std::string fileContent = filePart.substr(contentStart + 4);

        // Remove trailing \r\n if present
        if (fileContent.length() >= 2 && fileContent.substr(fileContent.length() - 2) == "\r\n") {
            fileContent = fileContent.substr(0, fileContent.length() - 2);
        }

        // Validate file size
        if (fileContent.length() > 1024 * 1024) {
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::PAYLOAD_TOO_LARGE_413, req);
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::PAYLOAD_TOO_LARGE_413);
            return;
        }

        // Resolve upload path (nginx-style)
        std::string uploadPath = router::utils::StringUtils::resolvePath(location->upload_path, server_root);

        // Use resolved path
        const std::string filePath = uploadPath + "/" + filename;

        // Ensure upload directory exists
        std::filesystem::create_directories(uploadPath);

        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile) {
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
            return;
        }

        outFile.write(fileContent.c_str(), fileContent.length());
        outFile.close();

        // Success response - redirect to upload page
        res.setHeaders(http::LOCATION, "/upload.html");
        router::utils::HttpResponseBuilder::setSuccessResponse(res, createSuccessMessage(filename, "uploaded"), http::CONTENT_TYPE_TEXT, req);

    } catch (const std::exception&) {
        // router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
        router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
    }
}

// ***************** DELETE HANDLER ***************** //

void del(const Request& req, Response& res, const Location* location, const std::string& server_root) {
    try {
        // Validate location configuration
        if (!location || location->upload_path.empty()) {
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::FORBIDDEN_403, req);
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::FORBIDDEN_403);
            return;
        }

        const std::string_view filePathView = req.getPath();

        // Extract the expected upload path prefix from location
        std::string uploadPrefix = "/uploads";
        if (filePathView.length() < uploadPrefix.length() + 1 ||
            filePathView.substr(0, uploadPrefix.length() + 1) != uploadPrefix + "/") {
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        std::string filename = std::string(filePathView.substr(uploadPrefix.length() + 1));
        if (filename.empty()) {
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        // Resolve upload path (nginx-style)
        std::string uploadPath = router::utils::StringUtils::resolvePath(location->upload_path, server_root);

        // Use resolved path
        const std::string filePath = uploadPath + "/" + filename;

        // Check if file exists
        if (!std::filesystem::exists(filePath)) {
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404);
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
            return;
        }

        // Attempt deletion
        if (std::filesystem::remove(filePath)) {
            // Success response - redirect to upload page
            res.setHeaders(http::LOCATION, "/upload.html");
            router::utils::HttpResponseBuilder::setSuccessResponse(res, createSuccessMessage(filename, "deleted"), http::CONTENT_TYPE_TEXT, req);
        } else {
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
        }

    } catch (const std::filesystem::filesystem_error&) {
        router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
        // router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
    } catch (const std::exception&) {
        router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
        // router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
    }
}

// ***************** CGI HANDLER ***************** //

/** Handle CGI requests for executable scripts */
void cgi(const Request& req, Response& res, const Location* location, const std::string& server_root, const Server* server) {
    try {
        // 1. Server and config Validation Phase
        if (!router::utils::isValidLocationServer(res, location, server)) {
            return;
        }

        // 2. Path Resolution Phase
        // Extract and validate file path
        std::string_view filePathView = req.getPath();
        if (!router::utils::isValidPath(filePathView, res)) {
            return;
        }

        // 3. File Existence and Executability Phase
        std::string filePath = router::utils::StringUtils::determineFilePathCGI(filePathView, location, server_root);
        if (!router::utils::isFileExistsAndExecutable(filePath, res)) {
            return;
        }


        // 4. File Validation Phase
        if (!router::utils::isCgiScriptWithLocation(filePath, location)) {
            // Not a CGI script, handle as regular file
            std::string fileContent = router::utils::FileUtils::readFileToString(filePath);
            std::string contentType = router::utils::FileUtils::getContentType(filePath);
            router::utils::HttpResponseBuilder::setSuccessResponse(res, fileContent, contentType, req);
            return;
        }

        // 5. CGI Execution Phase
        // READ: https://www.rfc-editor.org/rfc/rfc3875

        std::string scriptName = std::string(req.getPath());
        // Remove query string if present
        size_t queryPos = scriptName.find('?');
        if (queryPos != std::string::npos) {
            scriptName = scriptName.substr(0, queryPos);
        }

        // 5.1. CGI Environment Setup
        auto env = router::utils::setupCgiEnvironment(req, filePath, scriptName, *server);

        // DEBUG: Print all environment variables
        std::cout << "=== CGI Environment Variables ===" << std::endl;
        for (const auto& envVar : env) {
            std::cout << envVar << std::endl;
        }
        std::cout << "=================================" << std::endl;
        // END DEBUG

        // 5.2. Get and process request body for CGI input
        std::string body = std::string(req.getBody());

        // 5.3. Unchunk the body if it's chunked
        if (router::utils::isChunked(req)) {
            body = router::utils::parseChunkedRequestBody(body);
        }

        // 5.4. Execute CGI script and parse output
        CgiResult cgiResult = executeAndParseCgiScript(filePath, env, body);
        if (!cgiResult.success) {
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
            return;
        }

        // 6. Set response from parsed CGI result
        // 6.1. Set response status from CGI output
        res.setStatus(cgiResult.status);

        // 6.2. Set headers from CGI output
        for (const auto& [headerName, headerValue] : cgiResult.headers) {
            res.setHeaders(headerName, headerValue);
        }

        // 6.3. Set default content type if not specified
        auto contentType = res.getHeaders("Content-Type");
        if (contentType.empty()) {
            res.setHeaders(http::CONTENT_TYPE, http::CONTENT_TYPE_HTML);
        }

        // 6.4. Set connection header based on keep-alive logic
        if (router::utils::shouldKeepAlive(req)) {
            res.setHeaders(http::CONNECTION, http::CONNECTION_KEEP_ALIVE);
        } else {
            res.setHeaders(http::CONNECTION, http::CONNECTION_CLOSE);
        }

        // 6.5. Set response body
        res.setBody(cgiResult.body);

        // 6.6. Set Content-Length header based on body size
        res.setHeaders(http::CONTENT_LENGTH, std::to_string(cgiResult.body.length()));

    } catch (const std::runtime_error& e) {
        // 7. File not found or read error
        router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404);
        // router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
    } catch (const std::exception& e) {
        router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
        // router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
    }
}

// ***************** REDIRECT HANDLER ***************** //

/** Handle HTTP redirection requests */
void redirect(const Request& req, Response& res, const Location* location) {
    try {
        // Validate location configuration
        if (!location || location->return_url.empty()) {
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404);
            // router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
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
        res.setHeaders(http::LOCATION, redirectUrl);

        // Set connection header based on keep-alive logic
        if (router::utils::shouldKeepAlive(req)) {
            res.setHeaders(http::CONNECTION, http::CONNECTION_KEEP_ALIVE);
        } else {
            res.setHeaders(http::CONNECTION, http::CONNECTION_CLOSE);
        }

        // Optional: Add a simple HTML body for browsers that don't follow redirects automatically
        std::string body = "<!DOCTYPE html><html><head><title>Redirecting...</title></head><body>";
        body += "<p>If you are not redirected automatically, <a href=\"" + redirectUrl + "\">click here</a>.</p>";
        body += "</body></html>";

        res.setBody(body);

        // Set Content-Type header
        res.setHeaders(http::CONTENT_TYPE, http::CONTENT_TYPE_HTML);

    } catch (const std::exception& e) {
        // Unexpected error
        // router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
        router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
    }
}


