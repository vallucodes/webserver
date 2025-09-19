/**
 * @file Handlers.hpp
 * @brief HTTP Request Handler Functions
 */

#pragma once

#include <string>

#include "../../request/Request.hpp"
#include "../../response/Response.hpp"
#include "../../router/Router.hpp"
#include "../HttpConstants.hpp"

/** Core HTTP Request Handler Functions */

// Now using router::utils::FileUtils::readFileToString instead

/** Handle GET requests for static files and pages */
void get(const Request& req, Response& res, const Location* location = nullptr);

/** Handle POST requests for file uploads */
void post(const Request& req, Response& res, const Location* location = nullptr);

/**
 * @brief Handle DELETE requests for file removal
 * @param req HTTP request with file path
 * @param res Response object
 * @param location Location configuration
 */
void del(const Request& req, Response& res, const Location* location = nullptr);

/**
 * @brief Handle CGI requests for executable scripts
 * @param req HTTP request
 * @param res Response object
 * @param location Location configuration
 */
void cgi(const Request& req, Response& res, const Location* location = nullptr);

/**
 * @brief Handle HTTP redirection requests
 * @param req HTTP request
 * @param res Response object
 * @param location Location configuration
 */
void redirect(const Request& req, Response& res, const Location* location = nullptr);

/**
 * @brief Check if file is CGI script
 * @param filename Filename to check
 * @param location Location configuration
 * @return true if CGI script
 */
bool isCgiScriptWithLocation(const std::string& filename, const Location* location);
