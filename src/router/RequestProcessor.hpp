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

    /** Validate request path for security issues */
    // bool validatePath(const std::string& path) const;

    /** Normalize request path for processing */
    // void normalizePath(std::string& path) const;

  private:
    /** Execute handler with error handling */
    bool executeHandler(const Handler* handler,
                          const Request& req, Response& res,
                          const Server& server) const;

    /** Try to serve request as static file */
    bool tryServeAsStaticFile(const Request& req, Response& res,
                               const std::string& method, const Server& server) const;

};
