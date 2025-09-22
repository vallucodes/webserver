/**
 * @file HttpResponseBuilder.hpp
 * @brief Error response building utilities
 */

#pragma once

#include <string> // for std::string
#include "../HttpConstants.hpp"

// Forward declarations
class Response;

namespace router {
namespace utils {

/** Utility class for building error and success responses */
class HttpResponseBuilder {
    public:
        /** Set a complete error response with appropriate status, headers, and body */
        static void setErrorResponse(Response& res, int status);

        /** Set a success response with content and content type */
        static void setSuccessResponse(Response& res, const std::string& content, const std::string& contentType);

        /** Get HTML content for default error pages */
        static std::string getErrorPageHtml(int status);

    private:
        HttpResponseBuilder() = delete; // Static class
};

} // namespace utils
} // namespace router
