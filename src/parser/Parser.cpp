#include "Parser.hpp"

void parseRequestLineFormat(Request& req, const std::string& firstLineStr){
    std::istringstream lineStream(firstLineStr);
    std::string method, path, version;
    lineStream >> method >> path >> version;
    req.setMethod(method);
    req.setPath(path);
    req.setHttpVersion(version);
}


void isValidMethod(std::string_view method) {
    static const std::unordered_set<std::string_view> validMethods = {
        "GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH"
    };
    if (validMethods.find(method) == validMethods.end())
        throw std::runtime_error("Invalid request: method not found");
}

void isValidRequestTarget(std::string_view method, std::string_view path){
    if (path.empty()) {
    throw std::runtime_error("Invalid request: empty target");
    }

    if (method != "CONNECT" && method != "OPTIONS" && path[0] != '/') {
        throw std::runtime_error("Invalid request target: must start with '/'");
    }
    for (char c : path) {
        if (c <= 0x1F || c == 0x7F || c == ' ') {
            throw std::runtime_error("Invalid character in request target");
        }
    }

}

void isValidProtocol(std::string_view protocol){
    if (protocol != "HTTP/1.1" && protocol != "HTTP/1.0")
            throw std::runtime_error("Invalid HTTP protocol");
}

bool validateRequestLineFormat(const Request& req){
    isValidMethod(req.getMethod());
    isValidRequestTarget(req.getMethod(), req.getPath());
    isValidProtocol(req.getHttpVersion());
    return true;
}

void parseHeader(Request& req, const std::string_view& headerLines){
    size_t pos = 0;
    while (true){
        size_t lineEnd = headerLines.find("\r\n", pos);
        if (lineEnd == std::string_view::npos || lineEnd == pos)
            break;
        std::string_view line = headerLines.substr(pos, lineEnd - pos);
        size_t colon = line.find(":");
        if (colon != std::string_view::npos){
            req.setHeaders(trim(line.substr(0, colon)), trim(line.substr(colon + 1)));
        }
        pos = lineEnd + 2;
    };
}


Request Parser::parseRequest(const std::string& httpString) {
    Request req;
    std::string_view sv(httpString);

    //Parse request line
    size_t pos = sv.find("\r\n");
    std::string_view firstLine = sv.substr(0, pos);
    std::string firstLineStr(firstLine);
    parseRequestLineFormat(req, firstLineStr);
    if (validateRequestLineFormat(req) != 0)
        //do something

    //Parse headers
    size_t posEndHeader = sv.find("\r\n\r\n");
    std::string_view headerLines = sv.substr(pos + 2, (posEndHeader + 2) - (pos + 2));
    parseHeader(req, headerLines);

    //parse body
    auto contentLengthIt = req.getHeaders("Content-Length");
    if (!contentLengthIt.empty()) {
        size_t contentLength = std::stoul(std::string(contentLengthIt));
        std::string_view body = sv.substr(posEndHeader + 4, contentLength);
        req.setBody(std::string(body));
    }

    req.print();
    return req;
}


std::string Parser::serializeResponse(const Response& response){

}