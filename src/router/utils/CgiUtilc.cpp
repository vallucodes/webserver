#include "CgiUtilc.hpp"
#include "../../server/Server.hpp"

namespace router {
namespace utils {

bool isCgiScriptWithLocation(const std::string& filename, const Location* location) {
    if (!location || location->cgi_ext.empty()) {
        return false;
    }

    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        // Get the extension of the filename
        std::string ext = filename.substr(dotPos + 1);
        // Convert to lowercase for case-insensitive comparison
        for (char& c : ext) {
            c = std::tolower(c);
        }

        // Check against location-configured extensions
        for (const auto& allowedExt : location->cgi_ext) {
            std::string allowedExtLower = allowedExt;
            // Convert to lowercase for case-insensitive comparison
            for (char& c : allowedExtLower) {
                c = std::tolower(c);
            }
            // Remove leading dot from allowed extension for comparison
            if (allowedExtLower.length() > 0 && allowedExtLower[0] == '.') {
                allowedExtLower = allowedExtLower.substr(1);
            }
            if (ext == allowedExtLower) {
                return true;
            }
        }
    }
    return false;
}

/** Parse chunked request body */
std::string parseChunkedRequestBody(const std::string& body) {
    std::string result;
    size_t pos = 0;

    while (pos < body.length()) {
        // Find the end of the chunk size line
        size_t lineEnd = body.find("\r\n", pos);
        if (lineEnd == std::string::npos) {
            lineEnd = body.find("\n", pos);
            if (lineEnd == std::string::npos) break;
        }

        // Extract chunk size
        std::string chunkSizeStr = body.substr(pos, lineEnd - pos);
        size_t chunkSize = 0;
        try {
            chunkSize = std::stoul(chunkSizeStr, nullptr, 16);
        } catch (...) {
            break; // Invalid chunk size
        }

        // If chunk size is 0, we're done
        if (chunkSize == 0) break;

        // Move past the chunk size line
        pos = lineEnd + (body[lineEnd] == '\r' ? 2 : 1);

        // Extract chunk data
        if (pos + chunkSize <= body.length()) {
            result += body.substr(pos, chunkSize);
            pos += chunkSize;

            // Skip the trailing CRLF
            if (pos + 2 <= body.length() && body.substr(pos, 2) == "\r\n") {
                pos += 2;
            } else if (pos + 1 <= body.length() && body[pos] == '\n') {
                pos += 1;
            }
        } else {
            break; // Invalid chunk
        }
    }

    return result;
}


/** Set up CGI environment variables */
std::vector<std::string> setupCgiEnvironment(const Request& req, const std::string& scriptPath, const std::string& scriptName, const Server& server) {
    std::vector<std::string> env;

    // Basic CGI environment variables
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env.push_back("REQUEST_METHOD=" + std::string(req.getMethod()));
    env.push_back("SCRIPT_NAME=" + scriptName);
    env.push_back("SCRIPT_FILENAME=" + scriptPath);

    // Fix PATH_INFO - should be the path portion after the script name
    std::string pathStr(req.getPath());
    size_t queryPos = pathStr.find('?');
    std::string pathWithoutQuery = (queryPos != std::string::npos) ? pathStr.substr(0, queryPos) : pathStr;

    // Extract PATH_INFO (path after script name)
    std::string pathInfo = "";
    if (pathWithoutQuery.length() > scriptName.length()) {
        pathInfo = pathWithoutQuery.substr(scriptName.length());
    }
    env.push_back("PATH_INFO=" + pathInfo);
    env.push_back("PATH_TRANSLATED=" + scriptPath + pathInfo);

    // Query string handling
    if (queryPos != std::string::npos) {
        env.push_back("QUERY_STRING=" + pathStr.substr(queryPos + 1));
    } else {
        env.push_back("QUERY_STRING=");
    }

    // Content handling - handle chunked requests
    std::string body = std::string(req.getBody());
    auto transferEncoding = req.getHeaders("transfer-encoding");
    bool isChunked = false;

    if (!transferEncoding.empty()) {
        for (const auto& encoding : transferEncoding) {
            if (encoding.find("chunked") != std::string::npos) {
                isChunked = true;
                break;
            }
        }
    }

    // Unchunk the body if it's chunked
    if (isChunked) {
        body = parseChunkedRequestBody(body);
    }

    auto contentType = req.getHeaders("content-type");
    if (!contentType.empty()) {
        env.push_back("CONTENT_TYPE=" + contentType[0]);
    }

    auto contentLength = req.getHeaders("content-length");
    if (!contentLength.empty()) {
        env.push_back("CONTENT_LENGTH=" + contentLength[0]);
    } else {
        // Use the actual body size (after unchunking if needed)
        env.push_back("CONTENT_LENGTH=" + std::to_string(body.length()));
    }

    // Server information - now using dynamic values from server config
    env.push_back("SERVER_SOFTWARE=webserv/1.0");
    env.push_back("SERVER_NAME=" + server.getName());
    env.push_back("SERVER_PORT=" + std::to_string(server.getPort()));

    // Remote client info (simplified)
    env.push_back("REMOTE_ADDR=127.0.0.1");
    env.push_back("REMOTE_HOST=localhost");

    // Add PATH for finding executables
    env.push_back("PATH=/usr/bin:/bin:/usr/local/bin");

    return env;
}

} // namespace utils
} // namespace router
