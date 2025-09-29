/**
 * @file RequestProcessor.hpp
 * @brief Request processing logic separated from Router
 */

#pragma once

#include <string> // for std::string
#include <functional> // for std::function
#include "../request/Request.hpp"
#include "../response/Response.hpp"
#include "../server/Server.hpp"
#include "HttpConstants.hpp"

struct Location;
using Handler = std::function<void(const Request&, Response&, const Server&)>;

/**
 * @class RequestProcessor
 * @brief Processes HTTP requests and coordinates with handlers
 */
class RequestProcessor {
  public:
    RequestProcessor();
    ~RequestProcessor();

    /** Process HTTP request with routing information */
    void processRequest(const Request& req, const Handler* handler,
                        Response& res, const Server& server) const;


  private:
    /** Execute handler with error handling */
    bool executeHandler(const Handler* handler,
                          const Request& req, Response& res,
                          const Server& server) const;

    /** Try to serve request as static file */
    bool tryServeAsStaticFile(const Request& req, Response& res,
                               const std::string& method, const Server& server) const;

    /** Check if path exists but method is not allowed */
    bool isPathExistsButMethodNotAllowed(const Request& req, const Server& server) const;

    /** Check if path matches any configured location */
    bool isPathConfigured(const Request& req, const Server& server) const;

    /** Find matching location configuration for a path */
    const Location* findLocationForPath(const Server& server, const std::string& path) const;

};
