#include "../../../inc/webserv.hpp"
#include "Handlers.hpp"
#include "../Router.hpp"

using namespace http;

#include <fstream>
#include <sstream>
#include <filesystem>
#include <cctype>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>

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
    res.setHeaders("Content-Type", contentType);
    res.setHeaders("Content-Length", std::to_string(contentLength));
    res.setHeaders("Connection", CONNECTION_CLOSE);
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
        const auto& contentTypeKey = req.getHeaders("content-type");

        if (contentTypeKey.empty()) {
            setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        std::string contentType = contentTypeKey.front();

        // Validate content type
        if (contentType.find("multipart/form-data") == std::string::npos) {
            setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        // Extract boundary
        const size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos == std::string::npos) {
            setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        const std::string boundary = "--" + contentType.substr(boundaryPos + 9);
        const std::string bodyStr(body);

        // Find file boundaries
        const size_t fileStart = bodyStr.find(boundary);
        if (fileStart == std::string::npos) {
            setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        const size_t fileEnd = bodyStr.find(boundary, fileStart + boundary.length());
        const std::string filePart = bodyStr.substr(fileStart, fileEnd - fileStart);

        // Extract filename
        const size_t filenamePos = filePart.find("filename=\"");
        if (filenamePos == std::string::npos) {
            setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        const size_t filenameEnd = filePart.find("\"", filenamePos + 10);
        if (filenameEnd == std::string::npos) {
            setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        std::string filename = sanitizeFilename(filePart.substr(filenamePos + 10, filenameEnd - filenamePos - 10));
        if (filename.empty()) {
            setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        // Extract file content
        const size_t contentStart = filePart.find("\r\n\r\n");
        if (contentStart == std::string::npos) {
            setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        std::string fileContent = filePart.substr(contentStart + 4);

        // Remove trailing \r\n if present
        if (fileContent.length() >= 2 && fileContent.substr(fileContent.length() - 2) == "\r\n") {
            fileContent = fileContent.substr(0, fileContent.length() - 2);
        }

        // Validate file size
        if (fileContent.length() > 1024 * 1024) {
            setErrorResponse(res, http::PAYLOAD_TOO_LARGE_413);
            return;
        }

        // Save file
        const std::string filePath = page::WWW + "/uploads/" + filename;

        // Ensure uploads directory exists
        std::filesystem::create_directories(page::WWW + "/uploads/");

        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile) {
            setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
            return;
        }

        outFile.write(fileContent.c_str(), fileContent.length());
        outFile.close();

        // Success response
        const std::string fileSizeStr = std::to_string(fileContent.length() / 1024.0).substr(0, 4) + " KB";
        setSuccessResponse(res, createSuccessHtml(filename, fileSizeStr), CONTENT_TYPE_HTML);

    } catch (const std::exception&) {
        setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
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
            setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        std::string filename = sanitizeFilename(std::string(filePathView.substr(9)));
        if (filename.empty()) {
            setErrorResponse(res, http::BAD_REQUEST_400);
            return;
        }

        const std::string filePath = page::WWW + "/uploads/" + filename;

        // Check if file exists
        if (!std::filesystem::exists(filePath)) {
            setErrorResponse(res, http::NOT_FOUND_404);
            return;
        }

        // Attempt deletion
        if (std::filesystem::remove(filePath)) {
            setSuccessResponse(res, createDeletionSuccessHtml(filename), CONTENT_TYPE_HTML);
        } else {
            setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
        }

    } catch (const std::filesystem::filesystem_error&) {
        setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
    } catch (const std::exception&) {
        setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
    }
}

// Check if file extension indicates CGI script
// @param filename The filename to check
// @return true if file should be handled by CGI
bool isCgiScript(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string ext = filename.substr(dotPos + 1);
        // Convert to lowercase for case-insensitive comparison
        for (char& c : ext) {
            c = std::tolower(c);
        }
        // CGI extensions
        // return ext == "py" || ext == "cgi" || ext == "ts" || ext == "js";
        return ext == "py" || ext == "cgi" || ext == "js";
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

// Handle CGI requests for executable scripts (e.g., .php files)
// @param req The incoming HTTP request
// @param res The response object to populate
void cgi(const Request& req, Response& res) {
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

        // Check if file exists and is executable
        if (!std::filesystem::exists(filePath)) {
            setErrorResponse(res, http::NOT_FOUND_404);
            return;
        }

        // Verify it's a CGI script
        if (!isCgiScript(filePath)) {
            // Not a CGI script, handle as regular file
            std::string fileContent = readFileToString(filePath);
            std::string contentType = getContentType(filePath);
            setSuccessResponse(res, fileContent, contentType);
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
            setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
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
        setErrorResponse(res, http::NOT_FOUND_404);
    } catch (const std::exception& e) {
        // Unexpected error
        setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
    }
}


