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
    if (method != "CONNECT" && method != "OPTIONS" && path[0] != '/')
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
    parseHeader(req, headerLines);
    //check for if http1.1 and no host == error!

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




// std::string Parser::serializeResponse(const Response& response){
// }