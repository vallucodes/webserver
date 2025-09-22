/**
 * @file HttpResponseBuilder.cpp
 * @brief Error response building utilities implementation
 */

#include "HttpResponseBuilder.hpp"
#include "../HttpConstants.hpp"
#include "../../response/Response.hpp"
#include "FileUtils.hpp"
#include <iostream> // for std::cout, std::endl

namespace router {
namespace utils {

void HttpResponseBuilder::setErrorResponse(Response& res, int status) {
    // Set the HTTP status line based on the error code
    if (status == http::NOT_FOUND_404) {
        res.setStatus(http::STATUS_NOT_FOUND_404);
    } else if (status == http::METHOD_NOT_ALLOWED_405) {
        res.setStatus(http::STATUS_METHOD_NOT_ALLOWED_405);
    } else if (status == http::BAD_REQUEST_400) {
        res.setStatus(http::STATUS_BAD_REQUEST_400);
    } else if (status == http::PAYLOAD_TOO_LARGE_413) {
        res.setStatus(http::STATUS_PAYLOAD_TOO_LARGE_413);
    } else if (status == http::FORBIDDEN_403) {
        res.setStatus(http::STATUS_FORBIDDEN_403);
    } else if (status == http::INTERNAL_SERVER_ERROR_500) {
        res.setStatus(http::STATUS_INTERNAL_SERVER_ERROR_500);
    } else {
        res.setStatus(http::STATUS_INTERNAL_SERVER_ERROR_500);
    }

    // Set standard headers for HTML error responses
    res.setHeaders(http::CONTENT_TYPE, http::CONTENT_TYPE_HTML);
    res.setHeaders(http::CONTENT_LENGTH, std::to_string(getErrorPageHtml(status).length()));
    res.setHeaders(http::CONNECTION, http::CONNECTION_CLOSE);

    // Set the response body with the error page HTML
    res.setBody(getErrorPageHtml(status));
}

void HttpResponseBuilder::setSuccessResponse(Response& res, const std::string& content, const std::string& contentType) {
    res.setStatus(http::STATUS_OK_200);
    res.setHeaders(http::CONTENT_TYPE, contentType);
    res.setHeaders(http::CONTENT_LENGTH, std::to_string(content.length()));
    res.setHeaders(http::CONNECTION, http::CONNECTION_CLOSE);
    res.setBody(content);
}

std::string HttpResponseBuilder::getErrorPageHtml(int status) {
    switch (status) {
        case http::NOT_FOUND_404:
            return FileUtils::readFileToString(error_page::ERROR_PAGE_NOT_FOUND_404);
        case http::METHOD_NOT_ALLOWED_405:
            return FileUtils::readFileToString(error_page::ERROR_PAGE_METHOD_NOT_ALLOWED_405);
        case http::BAD_REQUEST_400:
            return FileUtils::readFileToString(error_page::ERROR_PAGE_BAD_REQUEST_400);
        case http::PAYLOAD_TOO_LARGE_413:
            return FileUtils::readFileToString(error_page::ERROR_PAGE_PAYLOAD_TOO_LARGE_413);
        case http::INTERNAL_SERVER_ERROR_500:
            return FileUtils::readFileToString(error_page::ERROR_PAGE_INTERNAL_SERVER_ERROR_500);
        default:
            // Fallback to generic 500 error for unknown status codes
            return FileUtils::readFileToString(error_page::ERROR_PAGE_INTERNAL_SERVER_ERROR_500);
    }
}

} // namespace utils
} // namespace router
