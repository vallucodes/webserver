/**
 * @file Handlers.hpp
 * @brief HTTP Request Handler Functions
 */

#pragma once

#include <string> // for std::string
#include <vector> // for std::vector

#include "../../request/Request.hpp"
#include "../../response/Response.hpp"

// Forward declarations
struct Location;
class Server;

/** Core HTTP Request Handler Functions */

/** Handle GET requests for static files and pages */
void get(const Request& req, Response& res, const Server& server);

/** Handle POST requests for file uploads */
void post(const Request& req, Response& res, const Server& server);

/** Handle DELETE requests for file removal */
void del(const Request& req, Response& res, const Server& server);

/** Handle CGI requests for executable scripts */
void cgi(const Request& req, Response& res, const Server& server);

/** Handle HTTP redirection requests */
void redirect(const Request& req, Response& res, const Server& server);
