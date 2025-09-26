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


// ********************************************************************************************** //
// ************************************** GET HANDLER ******************************************* //
// ********************************************************************************************** //

/** Handle GET requests for static files */
void get(const Request& req, Response& res, const Server& server) {
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
       // Find matching location for autoindex configuration
       const Location* location = nullptr;
       for (const auto& loc : server.getLocations()) {
         if (requestPath.find(loc.location) == 0) {
           location = &loc;
           break;
         }
       }
       if (router::utils::handleDirectoryRequest(filePath, requestPath, location, res, req)) {
       return;
     }
     // No index file found and autoindex disabled
     router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
     return;
  }

  // Serve static file
  if (router::utils::serveStaticFile(filePath, res, req)) {
       return;
  }

  // File not found
  router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
  } catch (const std::runtime_error&) {
     router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
  } catch (const std::exception&) {
     router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
  }
}

// ********************************************************************************************** //
// ************************************** POST HANDLER ******************************************* //
// ********************************************************************************************** //

/** Handle POST requests for file uploads */
void post(const Request& req, Response& res, const Server& server) {
  try {
     // Extract server root from server struct
     const std::string& server_root = server.getRoot();

     // Find the appropriate location for this request
     std::string_view requestPath = req.getPath();
     const Location* location = nullptr;
     for (const auto& loc : server.getLocations()) {
       if (std::string(requestPath).find(loc.location) == 0 && !loc.upload_path.empty()) {
         location = &loc;
         break;
       }
     }

     // Validate location configuration
     if (!location || location->upload_path.empty()) {
       router::utils::HttpResponseBuilder::setErrorResponse(res, http::FORBIDDEN_403, req);
       return;
     }

     // Handle chunked request body if needed
     std::string processedBody = std::string(req.getBody());

     if (router::utils::isChunked(req)) {
         processedBody = router::utils::parseChunkedRequestBody(processedBody);
     }

     const auto& contentTypeKey = req.getHeaders("content-type");

     if (contentTypeKey.empty()) {
         router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
       // router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req, server);
       return;
     }

     std::string contentType = contentTypeKey.front();

     // Validate content type
     if (contentType.find("multipart/form-data") == std::string::npos) {
         router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
       return;
     }

     // Extract boundary
     const size_t boundaryPos = contentType.find("boundary=");
     if (boundaryPos == std::string::npos) {
       router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
       return;
  }

  const std::string boundary = "--" + contentType.substr(boundaryPos + 9);
  const std::string bodyStr(processedBody);

     // Find file boundaries
     const size_t fileStart = bodyStr.find(boundary);
     if (fileStart == std::string::npos) {
         router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
       return;
     }

     const size_t fileEnd = bodyStr.find(boundary, fileStart + boundary.length());
     const std::string filePart = bodyStr.substr(fileStart, fileEnd - fileStart);

     // Extract filename
     const size_t filenamePos = filePart.find("filename=\"");
     if (filenamePos == std::string::npos) {
       router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
       return;
     }

     const size_t filenameEnd = filePart.find("\"", filenamePos + 10);
     if (filenameEnd == std::string::npos) {
         router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
       return;
     }

     std::string filename = filePart.substr(filenamePos + 10, filenameEnd - filenamePos - 10);
     if (filename.empty()) {
         router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
       return;
     }

     // Extract file content
     const size_t contentStart = filePart.find("\r\n\r\n");
     if (contentStart == std::string::npos) {
         router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
       return;
     }

  std::string fileContent = filePart.substr(contentStart + 4);

     // Remove trailing \r\n if present
     if (fileContent.length() >= 2 && fileContent.substr(fileContent.length() - 2) == "\r\n") {
         fileContent = fileContent.substr(0, fileContent.length() - 2);
     }

     // Validate file size
     if (fileContent.length() > 1024 * 1024) {
       router::utils::HttpResponseBuilder::setErrorResponse(res, http::PAYLOAD_TOO_LARGE_413, req);
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
         router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
       return;
     }

     outFile.write(fileContent.c_str(), fileContent.length());
     outFile.close();

     // Success response - redirect to upload page
     res.setHeaders(http::LOCATION, "/upload.html");
     router::utils::HttpResponseBuilder::setCreatedResponse(res, router::utils::createSuccessMessage(filename, "uploaded"), http::CONTENT_TYPE_TEXT, req);

  } catch (const std::exception&) {
     router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
  }
}

// ********************************************************************************************** //
// ************************************** DELETE HANDLER **************************************** //
// ********************************************************************************************** //

/** Handle DELETE requests for file removal */
void del(const Request& req, Response& res, const Server& server) {
  try {
     // Extract server root from server struct
     const std::string& server_root = server.getRoot();

     // Find the appropriate location for this request
     std::string_view requestPath = req.getPath();
     const Location* location = nullptr;
     for (const auto& loc : server.getLocations()) {
       if (std::string(requestPath).find(loc.location) == 0 && !loc.upload_path.empty()) {
         location = &loc;
         break;
       }
     }

     // Validate location configuration
     if (!location || location->upload_path.empty()) {
       router::utils::HttpResponseBuilder::setErrorResponse(res, http::FORBIDDEN_403, req);
       return;
     }

     const std::string_view filePathView = req.getPath();

     // Extract the expected upload path prefix from location
     std::string uploadPrefix = "/uploads";
     if (filePathView.length() < uploadPrefix.length() + 1 ||
         filePathView.substr(0, uploadPrefix.length() + 1) != uploadPrefix + "/") {
       router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
       return;
     }

     std::string filename = std::string(filePathView.substr(uploadPrefix.length() + 1));
     if (filename.empty()) {
       router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
       return;
     }

     // Resolve upload path (nginx-style)
     std::string uploadPath = router::utils::StringUtils::resolvePath(location->upload_path, server_root);

     // Use resolved path
     const std::string filePath = uploadPath + "/" + filename;

     // Check if file exists
     if (!std::filesystem::exists(filePath)) {
         router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
       return;
     }

     // Attempt deletion
     if (std::filesystem::remove(filePath)) {
         // Success response - 204 No Content (file deleted successfully)
       router::utils::HttpResponseBuilder::setNoContentResponse(res, req);
     } else {
       router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
     }

  } catch (const std::filesystem::filesystem_error&) {
     router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
  } catch (const std::exception&) {
     router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
  }
}

// ********************************************************************************************** //
// ************************************** CGI HANDLER ******************************************* //
// ********************************************************************************************** //

/** Handle CGI requests for executable scripts */
void cgi(const Request& req, Response& res, const Server& server) {
  try {
     // Extract server root from server struct
     const std::string& server_root = server.getRoot();

     // Find the appropriate location for this request
     std::string_view requestPath = req.getPath();
     const Location* location = nullptr;
     for (const auto& loc : server.getLocations()) {
       if (std::string(requestPath).find(loc.location) == 0 && !loc.cgi_path.empty() && !loc.cgi_ext.empty()) {
         location = &loc;
         break;
       }
     }

     // 1. Server and config Validation Phase
    if (!router::utils::isValidLocationServer(res, location, server, req)) {
         return;
     }

     // 2. Path Resolution Phase
     // Extract and validate file path
     std::string_view filePathView = req.getPath();
     if (!router::utils::isValidPath(filePathView, res, req)) {
         return;
     }

     // 3. File Existence and Executability Phase
     std::string filePath = router::utils::StringUtils::determineFilePathCGI(filePathView, location, server_root);
     if (!router::utils::isFileExistsAndExecutable(filePath, res, req)) {
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
  auto env = router::utils::setupCgiEnvironment(req, filePath, scriptName, server);

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
         // If CGI failed, check if it's a timeout (504) or other error
        if (cgiResult.status.find("504") != std::string::npos) {
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::GATEWAY_TIMEOUT_504, req);
            return;
        }
        router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
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
     router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
  } catch (const std::exception& e) {
     router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
  }
}

// ********************************************************************************************** //
// ************************************** REDIRECT HANDLER ************************************** //
// ********************************************************************************************** //

/** Handle HTTP redirection requests */
void redirect(const Request& req, Response& res, const Server& server) {
  try {
  // Find the appropriate location for this request
  std::string_view requestPath = req.getPath();
  const Location* location = nullptr;
  for (const auto& loc : server.getLocations()) {
    if (std::string(requestPath).find(loc.location) == 0 && !loc.return_url.empty()) {
      location = &loc;
      break;
    }
  }

  // Validate location configuration
  if (!location || location->return_url.empty()) {
       router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
     return;
  }

  // Use location-configured redirect URL
  std::string redirectUrl = location->return_url;

  // Log the redirect for debugging
  std::cout << "REDIRECT: " << req.getPath() << " -> " << redirectUrl << std::endl;

  // Set redirect status code
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

  // Set Content-Length header
  res.setHeaders(http::CONTENT_LENGTH, std::to_string(body.length()));


  } catch (const std::exception& e) {
     // Unexpected error
     router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
  }
}


