#pragma once

#include <string>

#include "../../request/Request.hpp"
#include "../../response/Response.hpp"
#include "../../router/Router.hpp"

// Function declarations

// Read the contents of a file into a string
// @param filename Path to the file to read
// @return String containing the file contents
// @throws std::runtime_error if file cannot be opened or read
std::string readFileToString(const std::string& filename);

// Handle GET requests for static files and pages
// @param req The incoming HTTP request
// @param res The response object to populate
// @param location The matching location configuration (can be nullptr)
void get(const Request& req, Response& res, const Location* location = nullptr);

// Handle POST requests for file uploads
// @param req The incoming HTTP request containing multipart/form-data
// @param res The response object to populate
// @param location The matching location configuration (can be nullptr)
void post(const Request& req, Response& res, const Location* location = nullptr);

// Handle DELETE requests for removing uploaded files
// @param req The incoming HTTP request with file path in URL
// @param res The response object to populate
// @param location The matching location configuration (can be nullptr)
void del(const Request& req, Response& res, const Location* location = nullptr);

// Handle CGI requests for executable scripts (e.g., .php files)
// @param req The incoming HTTP request
// @param res The response object to populate
// @param location The matching location configuration (can be nullptr)
void cgi(const Request& req, Response& res, const Location* location = nullptr);

// Handle HTTP redirection requests
// @param req The incoming HTTP request
// @param res The response object to populate
// @param location The matching location configuration (can be nullptr)
void redirect(const Request& req, Response& res, const Location* location = nullptr);
