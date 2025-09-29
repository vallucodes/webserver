/**
 * @file HttpResponseBuilder.hpp
 * @brief Error response building utilities
 */

#pragma once

#include <string> // for std::string
#include "../HttpConstants.hpp"

// Forward declarations
class Response;
class Request;
class Server;

namespace router {
namespace utils {

/** Utility class for building error and success responses */
class HttpResponseBuilder {
    public:
        /** Set a complete error response with default error pages only */
        static void setErrorResponse(Response& res, int status, const class Request& req);

        /** Set a complete error response with server-level error pages */
        static void setErrorResponse(Response& res, int status, const class Request& req, const class Server& server);

        /** Set a 405 Method Not Allowed response with Allow header listing allowed methods (with keep-alive support) */
        static void setMethodNotAllowedResponse(Response& res, const std::vector<std::string>& allowedMethods, const class Request& req);

        /** Set a success response with content and content type (with keep-alive support) */
        static void setSuccessResponse(Response& res, const std::string& content, const std::string& contentType, const class Request& req);

        /** Set a 201 Created response with content and content type (with keep-alive support) */
        static void setCreatedResponse(Response& res, const std::string& content, const std::string& contentType, const class Request& req);

        /** Set a 204 No Content response with empty body (with keep-alive support) */
        static void setNoContentResponse(Response& res, const class Request& req);

        /** Get HTML content for default error pages */
        static std::string getErrorPageHtml(int status);

        /** Get HTML content for error pages with server configuration support */
        static std::string getErrorPageHtml(int status, const class Server& server);

        /** Generate a default error page HTML */
        static std::string makeDefaultErrorPage(int code, const std::string& reason);


        /** Convert string status to integer status code */
        static int parseStatusCodeFromString(const std::string& statusString);

    private:

        HttpResponseBuilder() = delete; // Static class
};

} // namespace utils
} // namespace router
