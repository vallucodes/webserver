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
void get(const Request& req, Response& res);

// Handle POST requests for file uploads
// @param req The incoming HTTP request containing multipart/form-data
// @param res The response object to populate
void post(const Request& req, Response& res);

// Handle DELETE requests for removing uploaded files
// @param req The incoming HTTP request with file path in URL
// @param res The response object to populate
void del(const Request& req, Response& res);
