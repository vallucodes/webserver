/**
 * @file Handlers.hpp
 * @brief HTTP Request Handler Functions for the Web Server
 *
 * This header file declares the core handler functions that process different
 * types of HTTP requests in the web server. Each handler corresponds to a
 * specific HTTP method or functionality:
 *
 * Handler Functions:
 * - get():    Handles GET requests for static file serving
 * - post():   Handles POST requests for file uploads
 * - del():    Handles DELETE requests for file removal
 * - cgi():    Handles CGI script execution for dynamic content
 * - redirect(): Handles HTTP redirection responses
 *
 * Handler Architecture:
 * Each handler function follows the same signature pattern:
 *   void handler(const Request& req, Response& res, const Location* location)
 *
 * - Request&:  Contains all incoming HTTP request data (method, path, headers, body)
 * - Response&: Used to build the HTTP response (status, headers, body)
 * - Location*: Points to the matching server location configuration (routing rules)
 *
 * The handlers are registered with the Router based on server configuration
 * and HTTP method/path patterns. The Router invokes the appropriate handler
 * when a matching request is received.
 *
 * Error Handling:
 * Handlers should populate the Response object appropriately for both
 * success and error cases. The Router provides fallback error handling
 * if handlers throw exceptions.
 */

#pragma once

#include <string>

#include "../../request/Request.hpp"
#include "../../response/Response.hpp"
#include "../../router/Router.hpp"
#include "../HttpConstants.hpp"

/**
 * @brief Core HTTP Request Handler Functions
 *
 * This namespace contains all the handler function declarations used by the
 * web server's routing system. Each function handles a specific type of
 * HTTP request and is responsible for generating the appropriate response.
 */

/**
 * @brief Read the contents of a file into a string
 *
 * This utility function is used throughout the handlers to read file contents
 * for serving static files, error pages, and other file-based responses.
 *
 * The function handles file I/O operations and provides error handling for
 * common file access issues (file not found, permission denied, etc.).
 *
 * @param filename Path to the file to read (absolute or relative to working directory)
 * @return String containing the complete file contents
 * @throws std::runtime_error if file cannot be opened or read
 */
// Now using router::utils::FileUtils::readFileToString instead

/**
 * @brief Handle GET requests for static files and pages
 *
 * The GET handler is the most commonly used handler in the web server.
 * It serves static content including:
 * - HTML pages and templates
 * - CSS stylesheets
 * - JavaScript files
 * - Images and media files
 * - Any other static resources
 *
 * Functionality:
 * 1. Resolves the requested path to an actual file on disk
 * 2. Checks file permissions and existence
 * 3. Determines appropriate MIME type based on file extension
 * 4. Sets proper HTTP headers (Content-Type, Content-Length, etc.)
 * 5. Streams or serves the file content
 * 6. Handles directory requests (serves index.html if configured)
 *
 * Path Resolution:
 * - Uses the location configuration to map URL paths to filesystem paths
 * - Supports root directory configuration
 * - Handles path traversal security (prevents "../" attacks)
 *
 * Error Handling:
 * - Returns 404 for non-existent files
 * - Returns 403 for permission denied
 * - Returns 500 for internal server errors
 *
 * @param req The incoming HTTP request containing the URL path
 * @param res The response object to populate with file content and headers
 * @param location The matching location configuration (contains root path, index files, etc.)
 */
void get(const Request& req, Response& res, const Location* location = nullptr);

/**
 * @brief Handle POST requests for file uploads
 *
 * The POST handler manages file upload functionality through HTTP POST requests.
 * It supports multipart/form-data encoding for uploading files to the server.
 *
 * Key Features:
 * - Parses multipart/form-data request bodies
 * - Extracts uploaded files and form data
 * - Validates file types and sizes (if configured)
 * - Saves uploaded files to configured upload directory
 * - Provides upload progress and status feedback
 *
 * Multipart Processing:
 * 1. Parses Content-Type header to identify boundary string
 * 2. Splits request body into parts using boundary delimiters
 * 3. Extracts headers and content for each part
 * 4. Identifies file uploads vs. regular form fields
 * 5. Saves files with appropriate names and permissions
 *
 * Security Considerations:
 * - Validates upload_path configuration in location
 * - Prevents directory traversal attacks
 * - Checks file size limits
 * - Sanitizes filenames
 *
 * Response Handling:
 * - Returns 201 Created on successful upload
 * - Returns 400 Bad Request for malformed data
 * - Returns 413 Payload Too Large for oversized files
 * - Returns 403 Forbidden if uploads not allowed
 *
 * @param req The incoming HTTP request containing multipart/form-data
 * @param res The response object to populate with upload results
 * @param location The matching location configuration (must have upload_path configured)
 */
void post(const Request& req, Response& res, const Location* location = nullptr);

/**
 * @brief Handle DELETE requests for removing uploaded files
 *
 * The DELETE handler provides file deletion functionality for uploaded files.
 * It allows clients to remove files that were previously uploaded via POST.
 *
 * Functionality:
 * 1. Extracts the file path from the URL
 * 2. Validates the path is within the upload directory
 * 3. Checks file existence and permissions
 * 4. Safely deletes the file from the filesystem
 * 5. Returns appropriate success/error responses
 *
 * Security Features:
 * - Path validation to prevent directory traversal
 * - Ensures deletion is only allowed within upload directories
 * - Validates file ownership/permissions
 * - Prevents deletion of system files or directories
 *
 * Use Cases:
 * - Cleanup after file processing
 * - User-initiated file removal
 * - Temporary file management
 * - Resource cleanup operations
 *
 * Response Codes:
 * - 204 No Content: File successfully deleted
 * - 404 Not Found: File doesn't exist
 * - 403 Forbidden: Deletion not allowed or insufficient permissions
 * - 500 Internal Server Error: Filesystem error during deletion
 *
 * @param req The incoming HTTP request with file path in URL
 * @param res The response object to populate with deletion results
 * @param location The matching location configuration (contains upload_path settings)
 */
void del(const Request& req, Response& res, const Location* location = nullptr);

/**
 * @brief Handle CGI requests for executable scripts (e.g., .php files)
 *
 * The CGI handler enables dynamic content generation by executing external
 * programs or scripts. It supports the Common Gateway Interface (CGI) standard
 * for web servers to interface with external programs.
 *
 * CGI Functionality:
 * 1. Identifies script files based on file extension (.py, .php, .pl, etc.)
 * 2. Prepares CGI environment variables (REQUEST_METHOD, QUERY_STRING, etc.)
 * 3. Executes the script as a child process
 * 4. Captures script output (headers and body)
 * 5. Parses CGI headers from script output
 * 6. Returns processed response to client
 *
 * Environment Variables Set:
 * - REQUEST_METHOD: HTTP method (GET, POST, etc.)
 * - QUERY_STRING: URL query parameters
 * - CONTENT_TYPE: Request content type
 * - CONTENT_LENGTH: Request body length
 * - SCRIPT_NAME: Script path
 * - SERVER_NAME, SERVER_PORT: Server information
 * - HTTP_* headers: All HTTP request headers
 *
 * Script Execution:
 * - Forks a child process to run the script
 * - Redirects stdin/stdout for communication
 * - Sends request body to script's stdin (for POST)
 * - Reads script response from stdout
 * - Handles timeouts and process cleanup
 *
 * Security Considerations:
 * - Validates script_path configuration
 * - Restricts executable permissions
 * - Prevents execution of unauthorized scripts
 * - Sanitizes environment variables
 *
 * Response Processing:
 * - Parses CGI headers (Status, Content-Type, etc.)
 * - Separates headers from body content
 * - Handles script errors gracefully
 * - Returns 500 error for script failures
 *
 * @param req The incoming HTTP request to be processed by the CGI script
 * @param res The response object to populate with script output
 * @param location The matching location configuration (must have cgi_path and cgi_ext)
 */
void cgi(const Request& req, Response& res, const Location* location = nullptr);

/**
 * @brief Handle HTTP redirection requests
 *
 * The redirect handler manages HTTP redirection responses, allowing the server
 * to redirect clients to different URLs. This is commonly used for:
 * - URL normalization and canonicalization
 * - Temporary redirects during maintenance
 * - Permanent redirects for moved resources
 * - Protocol upgrades (HTTP to HTTPS)
 * - SEO-friendly URL restructuring
 *
 * Redirection Types:
 * - 301 Moved Permanently: Resource permanently moved to new URL
 * - 302 Found: Temporary redirect (client should continue using original URL)
 * - 303 See Other: Redirect after POST to prevent form resubmission
 * - 307 Temporary Redirect: Temporary redirect preserving HTTP method
 * - 308 Permanent Redirect: Permanent redirect preserving HTTP method
 *
 * Functionality:
 * 1. Determines redirect type based on location configuration
 * 2. Sets appropriate HTTP status code (301, 302, etc.)
 * 3. Sets Location header with target URL
 * 4. Optionally includes redirect message in response body
 * 5. Handles relative and absolute redirect URLs
 *
 * URL Resolution:
 * - Supports absolute URLs (http://example.com/new-path)
 * - Supports relative URLs (/new-path, ../path)
 * - Handles protocol-relative URLs (//example.com/path)
 * - Resolves relative to current request context
 *
 * Configuration:
 * - Redirect URLs configured in location blocks
 * - Support for different redirect codes per location
 * - Can be combined with other location rules
 *
 * Security:
 * - Validates redirect URLs to prevent open redirects
 * - Prevents redirect loops
 * - Sanitizes user input in redirect URLs
 *
 * @param req The incoming HTTP request being redirected
 * @param res The response object to populate with redirect information
 * @param location The matching location configuration (contains redirect_url and redirect_code)
 */
void redirect(const Request& req, Response& res, const Location* location = nullptr);

/**
 * @brief Check if file extension indicates CGI script using location configuration
 * @param filename The filename to check
 * @param location The location configuration containing allowed CGI extensions
 * @return true if file should be handled by CGI
 */
bool isCgiScriptWithLocation(const std::string& filename, const Location* location);
