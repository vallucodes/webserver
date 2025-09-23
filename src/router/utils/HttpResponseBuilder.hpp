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

namespace router {
namespace utils {

/** Utility class for building error and success responses */
class HttpResponseBuilder {
    public:
        /** Set a complete error response with appropriate status, headers, and body */
        static void setErrorResponse(Response& res, int status);

        /** Set a complete error response with appropriate status, headers, and body (with keep-alive support) */
        static void setErrorResponse(Response& res, int status, const class Request& req);

        /** Set a success response with content and content type */
        static void setSuccessResponse(Response& res, const std::string& content, const std::string& contentType);

        /** Set a success response with content and content type (with keep-alive support) */
        static void setSuccessResponse(Response& res, const std::string& content, const std::string& contentType, const class Request& req);

        /** Get HTML content for default error pages */
        static std::string getErrorPageHtml(int status);

    private:
        HttpResponseBuilder() = delete; // Static class
};

} // namespace utils
} // namespace router
