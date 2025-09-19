/**
 * @file ErrorResponseBuilder.hpp
 * @brief Error response building utilities
 */

#pragma once

#include <string>
#include "../HttpConstants.hpp"

// Forward declarations
class Response;

namespace router {
namespace utils {

/**
 * @class ErrorResponseBuilder
 * @brief Utility class for building error and success responses
 */
class ErrorResponseBuilder {
public:
    /**
     * @brief Set a complete error response with appropriate status, headers, and body
     * @param res Response object to configure with error details
     * @param status HTTP status code for the error (e.g., 404, 500)
     */
    static void setErrorResponse(Response& res, int status);

    /**
     * @brief Set a success response with content and content type
     * @param res Response object to configure
     * @param content The response body content
     * @param contentType MIME type for the response content
     */
    static void setSuccessResponse(Response& res, const std::string& content, const std::string& contentType);

    /**
     * @brief Get HTML content for default error pages
     * @param status HTTP status code (e.g., 404, 500)
     * @return HTML content string for the error page
     */
    static std::string getErrorPageHtml(int status);

private:
    ErrorResponseBuilder() = delete; // Static class
};

} // namespace utils
} // namespace router
