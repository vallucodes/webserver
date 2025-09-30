/**
 * @file Handlers.cpp
 * @brief Implementation of HTTP request handlers
 */

#include "Handlers.hpp"
#include "HandlerUtils.hpp"
#include "MultipartParser.hpp"
#include "../utils/StringUtils.hpp"
#include "../utils/FileUtils.hpp"
#include "../utils/HttpResponseBuilder.hpp"
#include "../utils/ValidationUtils.hpp"
#include "../handlers/CgiExecutor.hpp"
#include "../utils/Utils.hpp"
#include "../HttpConstants.hpp"
#include "../../server/Server.hpp"
#include <sstream>
#include <filesystem> // for std::filesystem::directory_iterator, std::filesystem::path, std::filesystem::exists, std::filesystem::is_directory, std::filesystem::is_regular_file, std::filesystem::create_directories, std::filesystem::remove, std::filesystem::file_size, std::filesystem::last_write_time

using namespace http;


// ********************************************************************************************** //
// ************************************** GET HANDLER ******************************************* //
// ********************************************************************************************** //

/** Handle GET requests for static files */
void get(const Request& req, Response& res, const Server& server) {
  try {
    // 1. Extract and validate file path
    const std::string_view filePathView = req.getPath();
    if (filePathView.empty()) {
      router::handlers::HandlerUtils::setErrorResponse(res, http::NOT_FOUND_404, req, server);
      return;
    }

    // 2. Build file path
    const std::string requestPath = std::string(filePathView);
    std::string filePath;
    if (requestPath == page::ROOT_HTML || requestPath == page::INDEX_HTML_PATH) {
      filePath = server.getRoot() + page::INDEX_HTML_PATH;
    } else {
      filePath = server.getRoot() + requestPath;
    }

    // 3. Handle directory requests
    if (std::filesystem::is_directory(filePath)) {
      const Location* location = router::handlers::HandlerUtils::findBestMatchingLocation(requestPath, server);

      if (router::utils::handleDirectoryRequest(filePath, requestPath, location, res, req, server.getRoot())) {
        return;
      }

      // No index file found and autoindex disabled, or autoindex template loading failed
      if (location && location->autoindex) {
        router::handlers::HandlerUtils::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req, server);
      } else {
        router::handlers::HandlerUtils::setErrorResponse(res, http::NOT_FOUND_404, req, server);
      }
      return;
    }

    // 4. Serve static file
    if (router::utils::serveStaticFile(filePath, res, req)) {
      return;
    }

    // 5. File not found
    router::handlers::HandlerUtils::setErrorResponse(res, http::NOT_FOUND_404, req, server);

  } catch (const std::runtime_error&) {
    router::handlers::HandlerUtils::setErrorResponse(res, http::NOT_FOUND_404, req, server);
  } catch (const std::exception&) {
    router::handlers::HandlerUtils::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req, server);
  }
}

// ********************************************************************************************** //
// ************************************** POST HANDLER ******************************************* //
// ********************************************************************************************** //

/** Handle POST requests for file uploads */
void post(const Request& req, Response& res, const Server& server) {
  try {
    // 1. Find upload location
    const std::string requestPath(req.getPath());
    const Location* location = router::handlers::HandlerUtils::findUploadLocation(requestPath, server);
    if (!location || location->upload_path.empty()) {
      router::handlers::HandlerUtils::setErrorResponse(res, http::FORBIDDEN_403, req, server);
      return;
    }

    // 2. Process request body
    const std::string processedBody = router::handlers::HandlerUtils::processRequestBody(req);

    // 3. Validate content type
    const auto& contentTypeKey = req.getHeaders("content-type");
    if (!router::handlers::HandlerUtils::validateContentType(contentTypeKey, "multipart/form-data")) {
      router::handlers::HandlerUtils::setErrorResponse(res, http::BAD_REQUEST_400, req, server);
      return;
    }

    // 4. Extract boundary
    const std::string contentType = contentTypeKey[0];
    const std::string boundary = router::handlers::HandlerUtils::extractBoundary(contentType);
    if (boundary.empty()) {
      router::handlers::HandlerUtils::setErrorResponse(res, http::BAD_REQUEST_400, req, server);
      return;
    }

    // 5. Parse multipart data
    const auto multipartData = router::handlers::MultipartParser::parseMultipartData(processedBody, boundary);
    if (!multipartData.isValid) {
      router::handlers::HandlerUtils::setErrorResponse(res, http::BAD_REQUEST_400, req, server);
      return;
    }

    // 6. Validate filename
    if (!router::handlers::HandlerUtils::isValidFilename(multipartData.filename)) {
      router::handlers::HandlerUtils::setErrorResponse(res, http::BAD_REQUEST_400, req, server);
      return;
    }

    // 7. Validate file size
    if (!router::handlers::HandlerUtils::isValidFileSize(multipartData.content.length())) {
      router::handlers::HandlerUtils::setErrorResponse(res, http::PAYLOAD_TOO_LARGE_413, req, server);
      return;
    }

    // 8. Write file to disk
    const std::string server_root = server.getRoot();
    const std::string filePath = router::handlers::HandlerUtils::resolveFilePath(multipartData.filename, location, server_root);

    if (!router::handlers::HandlerUtils::writeFileToDisk(filePath, multipartData.content)) {
      router::handlers::HandlerUtils::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req, server);
      return;
    }

    // 9. Success response
    router::handlers::HandlerUtils::setSuccessResponse(res, http::CREATED_201, req);

  } catch (const std::exception&) {
    router::handlers::HandlerUtils::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req, server);
  }
}

// ********************************************************************************************** //
// ************************************** DELETE HANDLER **************************************** //
// ********************************************************************************************** //

/** Handle DELETE requests for file removal */
void del(const Request& req, Response& res, const Server& server) {
  try {
    // 1. Find upload location
    const std::string requestPath(req.getPath());
    const Location* location = router::handlers::HandlerUtils::findUploadLocation(requestPath, server);
    if (!location || location->upload_path.empty()) {
      router::handlers::HandlerUtils::setErrorResponse(res, http::FORBIDDEN_403, req, server);
      return;
    }

    // 2. Extract filename from path
    const std::string_view filePathView = req.getPath();
    const std::string uploadPrefix = "/uploads";

    if (filePathView.length() < uploadPrefix.length() + 1 ||
        filePathView.substr(0, uploadPrefix.length() + 1) != uploadPrefix + "/") {
      router::handlers::HandlerUtils::setErrorResponse(res, http::BAD_REQUEST_400, req, server);
      return;
    }

    const std::string filename = std::string(filePathView.substr(uploadPrefix.length() + 1));
    if (!router::handlers::HandlerUtils::isValidFilename(filename)) {
      router::handlers::HandlerUtils::setErrorResponse(res, http::BAD_REQUEST_400, req, server);
      return;
    }

    // 3. Resolve file path
    const std::string server_root = server.getRoot();
    const std::string filePath = router::handlers::HandlerUtils::resolveFilePath(filename, location, server_root);

    // 4. Check if file exists
    if (!std::filesystem::exists(filePath)) {
      router::handlers::HandlerUtils::setErrorResponse(res, http::NOT_FOUND_404, req, server);
      return;
    }

    // 5. Attempt deletion
    if (router::handlers::HandlerUtils::deleteFileFromDisk(filePath)) {
      router::handlers::HandlerUtils::setSuccessResponse(res, http::OK_200, req);
    } else {
      router::handlers::HandlerUtils::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req, server);
    }

  } catch (const std::filesystem::filesystem_error&) {
    router::handlers::HandlerUtils::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req, server);
  } catch (const std::exception&) {
    router::handlers::HandlerUtils::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req, server);
  }
}

// ********************************************************************************************** //
// ************************************** CGI HANDLER ******************************************* //
// ********************************************************************************************** //

/** Handle CGI requests for executable scripts */
void cgi(const Request& req, Response& res, const Server& server) {
  try {
    // 1. Find CGI location
    const std::string requestPath(req.getPath());
    const Location* location = router::handlers::HandlerUtils::findCgiLocation(requestPath, server);

    // 2. Server and config Validation Phase
    if (!router::utils::isValidLocationServer(res, location, &server, req)) {
      return;
    }

    // 3. Path Resolution Phase
    const std::string_view filePathView = req.getPath();
    if (!router::utils::isValidPath(filePathView, res, req, server)) {
      return;
    }

    // 4. File Existence and Executability Phase
    const std::string server_root = server.getRoot();
    const std::string filePath = router::utils::StringUtils::determineFilePathCGI(filePathView, location, server_root);
    if (!router::utils::isFileExistsAndExecutable(filePath, res, req, server)) {
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
  // std::cout << "=== CGI Environment Variables ===" << std::endl;
  // for (const auto& envVar : env) {
  // std::cout << envVar << std::endl;
  // }
  // std::cout << "=================================" << std::endl;
  // END DEBUG

  // 5.2. Get and process request body for CGI input
  const std::string body = router::handlers::HandlerUtils::processRequestBody(req);

     // 5.4. Execute CGI script and parse output
     CgiResult cgiResult = executeAndParseCgiScript(filePath, env, body);
     if (!cgiResult.success) {
         // If CGI failed, check if it's a timeout (504) or other error
        if (cgiResult.status.find("504") != std::string::npos) {
            router::utils::HttpResponseBuilder::setErrorResponse(res, http::GATEWAY_TIMEOUT_504, req, server);
            return;
        }
        router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req, server);
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
     router::handlers::HandlerUtils::setConnectionHeaders(res, req);

     // 6.5. Set response body
     res.setBody(cgiResult.body);

     // 6.6. Set Content-Length header based on body size
     res.setHeaders(http::CONTENT_LENGTH, std::to_string(cgiResult.body.length()));

  } catch (const std::runtime_error& e) {
     // 7. File not found or read error
     router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req, server);
  } catch (const std::exception& e) {
     router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req, server);
  }
}

// ********************************************************************************************** //
// ************************************** REDIRECT HANDLER ************************************** //
// ********************************************************************************************** //

/** Handle HTTP redirection requests */
void redirect(const Request& req, Response& res, const Server& server) {
  try {
    // 1. Find redirect location
    const std::string requestPath(req.getPath());
    const Location* location = router::handlers::HandlerUtils::findRedirectLocation(requestPath, server);
    if (!location || location->return_url.empty()) {
      router::handlers::HandlerUtils::setErrorResponse(res, http::NOT_FOUND_404, req, server);
      return;
    }

    // 2. Use location-configured redirect URL
    const std::string redirectUrl = location->return_url;

    // 3. Log the redirect for debugging
    std::cout << "REDIRECT: " << req.getPath() << " -> " << redirectUrl << std::endl;

    // 4. Set redirect status code
    res.setStatus(http::STATUS_FOUND_302);

    // 5. Set Location header for redirection
    res.setHeaders(http::LOCATION, redirectUrl);

    // 6. Set connection headers
    router::handlers::HandlerUtils::setConnectionHeaders(res, req);

    // 7. Add HTML body for browsers that don't follow redirects automatically
    const std::string body = "<!DOCTYPE html><html><head><title>Redirecting...</title></head><body>"
                             "<p>If you are not redirected automatically, <a href=\"" + redirectUrl + "\">click here</a>.</p>"
                             "</body></html>";

    res.setBody(body);
    res.setHeaders(http::CONTENT_TYPE, http::CONTENT_TYPE_HTML);
    res.setHeaders(http::CONTENT_LENGTH, std::to_string(body.length()));

  } catch (const std::exception&) {
    router::handlers::HandlerUtils::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req, server);
  }
}


