#include "Parser.hpp"

bool isValidMethod(std::string_view method) {
    static const std::unordered_set<std::string_view> validMethods = {
        "GET", "POST", "DELETE"
    };
    if (validMethods.find(method) == validMethods.end())
        return false;
    return true;
}

int fromHex(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

bool decodeHexPair(char hi, char lo, unsigned char &out) {
    int hiVal = fromHex(hi);
    int loVal = fromHex(lo);
    if (hiVal < 0 || loVal < 0)
        return false;
    out = static_cast<unsigned char>((hiVal << 4) | loVal);
    return true;
}

bool isValidRequestTarget(std::string& path) {
    if (path.empty())
        return false;
    const std::string_view invalid =
        "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        "\x7F \"<>\\^`{}|";

    for (std::size_t i = 0; i < path.size();) {
        unsigned char c = path[i];
       if (c == '%') {
            if (i + 2 >= path.size())
                return false;
            unsigned char decoded;
            if (!decodeHexPair(path[i+1], path[i+2], decoded))
                return false;
            if (decoded >= 128 || invalid.find(decoded) != std::string_view::npos)
                return false;
            path.replace(i, 3, 1, decoded);
            ++i;
        } else {
            if (c >= 128 || invalid.find(c) != std::string_view::npos)
                return false;
            ++i;
        }
    }
    return true;
}

bool parseRequestLineFormat(Request& req, const std::string& firstLineStr){
    std::istringstream lineStream(firstLineStr);
    std::string method, path, version;
    if (!(lineStream >> method >> path >> version))
        return false;
    req.setMethod(method);
    if (!isValidRequestTarget(path))
        return false;
    req.setPath(path);
    req.setHttpVersion(version);
    return true;
}

bool isValidProtocol(std::string_view protocol){
    if (protocol != "HTTP/1.1" && protocol != "HTTP/1.0")
        return false;
    return true;
}

bool isBadRequest(const Request& req){
    // Don't reject unsupported methods at parsing level - let RequestProcessor handle with 405 and retur method not allowed
    // if ( !isValidMethod(req.getMethod()) )
    //     return true;
    // if ( !isValidRequestTarget(req.getPath()))
    //     return true;
    if ( !isValidProtocol(req.getHttpVersion()))
        return true;
    return false;
}

bool parseHeader(Request& req, const std::string_view& headerLines){
    size_t pos = 0;
    while (true){
        size_t lineEnd = headerLines.find("\r\n", pos);
        if (lineEnd == std::string_view::npos || lineEnd == pos)
            break;
        std::string_view line = headerLines.substr(pos, lineEnd - pos);
        size_t colon = line.find(":");
        if (colon != std::string_view::npos){
            std::string key = trim(line.substr(0, colon));
            for (size_t i = 0; i < key.size(); i++) {
                key[i] = std::tolower((unsigned char) key[i]);
            }
            req.setHeaders(key, trim(line.substr(colon + 1)));
        }
        else{
            req.setError(true);
            req.setStatus("400 Bad Request");
            return false;
        }
        pos = lineEnd + 2;
    };
    return true;
}

bool isBadHeader(Request& req) {
    static const std::unordered_set<std::string_view> uniqueHeaders = {
        "host",
        "content-length",
        "content-type",
        "authorization",
        "from",
        "max-forwards",
        "date",
        "expect",
        "user-agent",
        "referer",
        "origin",
        "if-modified-since",
        "if-unmodified-since",
        "last-modified",
        "etag",
        "if-match",
        "if-none-match",
        "if-range",
        "content-location",
        "content-encoding",
        "content-location",
    };

    const auto& headers = req.getAllHeaders();

    // Check if host is there, cause host is mandatory
    auto hostIt = headers.find("host");
    if (hostIt == headers.end() || hostIt->second.empty()) {
        return true;
    }

    // Check for duplicate unique headers
    for (const auto& [key, values] : headers) {
        if (uniqueHeaders.count(key) && values.size() > 1) {
            return true;
        }
    }
    return false;
}

bool isBadMethod(Request& req){
    if (req.getHttpVersion() == "HTTP/1.1"){
        const auto& hostValues = req.getHeaders("host");
        if (hostValues.empty())
            return true;
    }
    if (req.getMethod() == "POST"){
        const auto& contentLength = req.getHeaders("content-length");
        const auto& transferEncoding = req.getHeaders("transfer-encoding");
        // ILIA Added this
        // Allow POST without Content-Length or Transfer-Encoding if body is empty
        // This handles cases like POST with size 0 where Content-Length header is optional
        if (contentLength.empty() && transferEncoding.empty()) {
            // Check if body is actually empty (no data after headers)
            // For now, we'll be more lenient and allow POST without Content-Length
            // The actual body validation will be handled by the handlers
            // return true;
        }
    }
    if (req.getMethod() == "GET"){
        const auto& contentLength = req.getHeaders("content-length");
        const auto& transferEncoding = req.getHeaders("transfer-encoding");
        if (!contentLength.empty() && !transferEncoding.empty())
            return true;
    }
    return false;
}

bool isChunked(Request& req){
    auto transferEncoding = req.getHeaders("transfer-encoding");
    bool findChunk = !transferEncoding.empty() && transferEncoding.front() == "chunked";
    return findChunk;
}

// std::string decodeChunkedBody(std::string body){
//     std::string result;
//     size_t pos = 0;

//     while (pos < body.size()) {
//         size_t lineEnd = body.find("\r\n", pos);
//         if (lineEnd == std::string::npos) break;

//         std::string sizeLine = body.substr(pos, lineEnd - pos);
//         size_t chunkSize = 0;
//         std::istringstream(sizeLine) >> std::hex >> chunkSize;
//         pos = lineEnd + 2;

//         if (chunkSize == 0) break;

//         if (pos + chunkSize > body.size()) break;

//         result.append(body.substr(pos, chunkSize));
//         pos += chunkSize + 2;
//     }

//     return result;
// }

void findKeepAlive(const std::vector<std::string>& headers, bool& kick_me) {
    if (headers.empty()) {
        kick_me = false;
        return;
    }
    const std::string& value = headers.front();
    if (value == "keep-alive")
        kick_me = false;
    else if (value == "close")
        kick_me = true;
    else
        std::cout << "Error: Unreachable: findKeepAlive()\n";
}


Request Parser::parseRequest(const std::string& httpString, bool& kick_me, bool bad_request) {
    Request req;
    std::string_view sv(httpString);

    // std::cout << "\noriginal string is:" << httpString << std::endl;
    // std::cout << "end of string" <<  std::endl;

    //Parse request line
    // std::cout << "\n--------- incoming string ----------\n" << httpString << "\n---------- END ----------\n" << std::endl;
    if (bad_request)
    {
        req.setError(true);
        req.setStatus(httpString);
        return req;
    }
    size_t pos = sv.find("\r\n");
    if (pos == std::string_view::npos){
        req.setError(true);
        req.setStatus("400 Bad Request");
        return req;
    }
    std::string_view firstLine = sv.substr(0, pos);
    std::string firstLineStr(firstLine);
    if ( !parseRequestLineFormat(req, firstLineStr) ){
        req.setError(true);
        req.setStatus("400 Bad Request");
        return req;
    }
    req.setError(isBadRequest(req));
    if (req.getError()){
        req.setStatus("400 Bad Request");
        return req;
    }

    //Parse headers
    size_t posEndHeader = sv.find("\r\n\r\n");
    std::string_view headerLines = sv.substr(pos + 2, (posEndHeader + 2) - (pos + 2));
    if (!parseHeader(req, headerLines))
        return req;
    req.setError(isBadHeader(req));
    if (req.getError()){
        req.setStatus("400 Bad Request");
        return req;
    };
    findKeepAlive(req.getHeaders("connection"), kick_me);
    req.setError(isBadMethod(req));
    if (req.getError()) {
        return req;
    };
    //parse body
    std::string_view body = sv.substr(posEndHeader + 4);
    auto contentValues = req.getHeaders("content-length");
    req.setBody(std::string(body.substr(0)));

    // if (!contentValues.empty()) {
    //     size_t contentLength = std::stoul(contentValues.front());
    //     req.setBody(std::string(body.substr(0, contentLength)));
    // }
    // else if (isChunked(req))
    //     req.setBody(decodeChunkedBody(std::string(body)));

    // ELSE IF content-type chunked parse the body
    // else
    //     req.setBody("");

    // req.print();
    return req;
}




// std::string Parser::serializeResponse(const Response& response){
// }
