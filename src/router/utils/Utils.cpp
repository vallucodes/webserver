#include "Utils.hpp"
#include "../../server/Server.hpp"

namespace router {
namespace utils {

/** Check if the request is chunked */
bool isChunked(const Request& req) {
    // READ: https://www.rfc-editor.org/rfc/rfc7230#section-3.3.1
    auto transferEncoding = req.getHeaders("transfer-encoding");
    // example: transfer-encoding: chunked
    bool findChunk = false;
    if (!transferEncoding.empty()) {
        // example: transfer-encoding: chunked -> true
        for (const auto& encoding : transferEncoding) {
            std::string lowerEncoding = encoding;
            std::transform(lowerEncoding.begin(), lowerEncoding.end(), lowerEncoding.begin(), ::tolower);
            if (lowerEncoding.find("chunked") != std::string::npos) {
                findChunk = true;
                break;
            }
        }
    }
    return findChunk;
}

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
        // \r is carriage return, \n is line feed (CRLF)
        size_t lineEnd = body.find("\r\n", pos);
        if (lineEnd == std::string::npos) {
            lineEnd = body.find("\n", pos);
            if (lineEnd == std::string::npos) break;
        }

        // Extract chunk size
        std::string chunkSizeStr = body.substr(pos, lineEnd - pos);
        size_t chunkSize = 0;

        /*
        1f\r\n
        abcdefghijklmnopqrstuvwxyz123\r\n
        0\r\n
        \r\n
        */
        // example: 1f\r\n -> 31
        // read 31 bytes → "abcdefghijklmnopqrstuvwxyz123"
        // example: 0\r\n -> 0
        // read 0 bytes → ""
        try {
            // should be a valid hexadecimal number
            // convrt to unsigned long with base 16
            chunkSize = std::stoul(chunkSizeStr, nullptr, 16);
        } catch (...) {
            break; // Invalid chunk size
        }

        // If chunk size is 0, no more chunks
        if (chunkSize == 0) break;

        // Move past the chunk size line
        // start from the next line
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
    // example: "abcdefghijklmnopqrstuvwxyz123"
    return result;
}


/** Set up CGI environment variables */
// READ: https://datatracker.ietf.org/doc/html/rfc3875
std::vector<std::string> setupCgiEnvironment(const Request& req, const std::string& scriptPath, const std::string& scriptName, const Server& server) {
    std::vector<std::string> env;

    // Basic CGI environment variables
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env.push_back("REQUEST_METHOD=" + std::string(req.getMethod()));
    env.push_back("SCRIPT_NAME=" + scriptName);
    env.push_back("SCRIPT_FILENAME=" + scriptPath);

    // PATH_INFO and PATH_TRANSLATED
    std::string pathStr(req.getPath());
    // Find the position of the query string
    // example: /cgi-bin/script.py?name=Ilia&age=43
    size_t queryPos = pathStr.find('?');
    // If there is a query string, extract the path before it
    // example: /cgi-bin/script.py
    std::string pathWithoutQuery = (queryPos != std::string::npos) ? pathStr.substr(0, queryPos) : pathStr;

    // PATH_INFO - should be the path portion after the script name
    std::string pathInfo = "";
    // For /cgi-bin/hello.py, scriptName is "/cgi-bin/hello.py", so PATH_INFO should be empty
    // For /cgi-bin/script.py/extra/path, scriptName is "/cgi-bin/script.py", so PATH_INFO should be "/extra/path"
    if (pathWithoutQuery.length() > scriptName.length() &&
        pathWithoutQuery.substr(0, scriptName.length()) == scriptName) {
        pathInfo = pathWithoutQuery.substr(scriptName.length());
    }
    env.push_back("PATH_INFO=" + pathInfo);
    if (!pathInfo.empty()) {
        env.push_back("PATH_TRANSLATED=" + (scriptPath + pathInfo));
    }

    // Query string handling
    if (queryPos != std::string::npos) {
        // example: /cgi-bin/script.py?name=Ilia&age=43 -> name=Ilia&age=43
        env.push_back("QUERY_STRING=" + pathStr.substr(queryPos + 1));
    } else {
        // example: /cgi-bin/script.py -> ""
        env.push_back("QUERY_STRING=");
    }

    // Content handling - handle chunked requests
    std::string body = std::string(req.getBody());
    auto transferEncoding = req.getHeaders("transfer-encoding");
    // example: transfer-encoding: chunked
    bool isChunked = false;

    /*
    HTTP/1.1 200 OK
    Transfer-Encoding: chunked

    4\r\n
    Wiki\r\n
    5\r\n
    pedia\r\n
    0\r\n
    \r\n
    */
    if (!transferEncoding.empty()) {
        // example: transfer-encoding: chunked -> true
        for (const auto& encoding : transferEncoding) {
            // example: transfer-encoding: chunked -> true
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
        // Use the actual body size
        env.push_back("CONTENT_LENGTH=" + std::to_string(body.length()));
    }

    // Server information - using dynamic values from server config
    env.push_back("SERVER_SOFTWARE=webserv/1.0");
    env.push_back("SERVER_NAME=" + server.getName());
    env.push_back("SERVER_PORT=" + std::to_string(server.getPort()));

    // Remote client info (simplified)
    // env.push_back("REMOTE_ADDR=127.0.0.1");
    env.push_back("REMOTE_ADDR=127.0.0.1");

    // env.push_back("REMOTE_HOST=localhost");
    env.push_back("REMOTE_HOST=localhost");

    // Add PATH for finding executables
    env.push_back("PATH=/usr/bin:/bin:/usr/local/bin");

    return env;
}

} // namespace utils
} // namespace router
