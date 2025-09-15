#include "Parser.hpp"

bool parseRequestLineFormat(Request& req, const std::string& firstLineStr){
    std::istringstream lineStream(firstLineStr);
    std::string method, path, version;
    if (!(lineStream >> method >> path >> version))
        return false;
    req.setMethod(method);
    req.setPath(path);
    req.setHttpVersion(version);
    return true;
}


bool isValidMethod(std::string_view method) {
    static const std::unordered_set<std::string_view> validMethods = {
        "GET", "POST", "DELETE"
    };
    if (validMethods.find(method) == validMethods.end())
        return false;
    return true;
}

bool isValidRequestTarget(std::string_view method, std::string_view path){
    if (path.empty())
        return false;
    for (char c : path) {
        if (c <= 0x1F || c == 0x7F || c == ' ') {
            return false;
        }
    }
    return true;
}

bool isValidProtocol(std::string_view protocol){
    if (protocol != "HTTP/1.1" && protocol != "HTTP/1.0")
        return false;
    return true;
}

bool isBadRequest(const Request& req){
    if ( !isValidMethod(req.getMethod()) )
        return true;
    if ( !isValidRequestTarget(req.getMethod(), req.getPath()))
        return true;
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
        "if-modified-since",
        "if-unmodified-since",
        "retry-after",
        "location",
        "server",
        "last-modified",
        "etag",
        "content-location"
    };
    for (const auto& [key, values] : req.getAllHeaders()) {
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
        if (contentLength.empty() && transferEncoding.empty())
            return true; 
    }
    if (req.getMethod() == "GET"){
        const auto& contentLength = req.getHeaders("content-length");
        const auto& transferEncoding = req.getHeaders("transfer-encoding");
        if (!contentLength.empty() && !transferEncoding.empty())
            return true; 
    }
    return false;
}

void printEscaped(const std::string& s) {
    for (char c : s) {
        switch (c) {
            case '\r': std::cout << "\\r"; break;
            case '\n': std::cout << "\\n"; break;
            default: std::cout << c; break;
        }
    }
    std::cout << std::endl;
}

Request Parser::parseRequest(const std::string& httpString) {
    Request req;
    std::string_view sv(httpString);
    
    std::cout << "------this is the string--------" << std::endl;
    printEscaped(httpString);
    std::cout << "------this was the string--------" << std::endl;
    //Parse request line
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
    req.setError(isBadMethod(req));
    if (req.getError()) {
        return req;
    };

    //parse body
    auto contentValues = req.getHeaders("content-length");
    if (!contentValues.empty()) {
        size_t contentLength = std::stoul(contentValues.front());
        std::string_view body = sv.substr(posEndHeader + 4, contentLength);
        req.setBody(std::string(body));
    }
    else {
        contentValues = req.getHeaders("transfer-encoding");
        if (!contentValues.empty()){
            std::string_view body = sv.substr(posEndHeader + 4, std::string_view::npos);
            req.setBody(std::string(body));
        }
    }

    // req.print();
    return req;
}




// std::string Parser::serializeResponse(const Response& response){
// }