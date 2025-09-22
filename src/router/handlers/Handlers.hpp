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

/** Handle GET requests for static files and pages */
void get(const Request& req, Response& res, const Location* location = nullptr);

/** Handle POST requests for file uploads */
void post(const Request& req, Response& res, const Location* location = nullptr);

/** Handle DELETE requests for file removal */
void del(const Request& req, Response& res, const Location* location = nullptr);

/** Handle CGI requests for executable scripts */
void cgi(const Request& req, Response& res, const Location* location = nullptr);

/** Handle HTTP redirection requests */
void redirect(const Request& req, Response& res, const Location* location = nullptr);

/** Check if file is CGI script */
bool isCgiScriptWithLocation(const std::string& filename, const Location* location);
