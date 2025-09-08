#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

// Interface for HTTP method, path, and body handling
class AMessage {
  public:
    AMessage(void);
    virtual ~AMessage(void);

    // Setter methods for HTTP message components (GET, POST, etc.)
    virtual void setMethod(const std::string& method);
    virtual void setPath(const std::string& path);
    virtual void setBody(const std::string& body);
    virtual void setHeaders(const std::string& key, const std::string& value);
    virtual void setHttpVersion(const std::string& httpVersion);


    // Getter methods for HTTP message components
    virtual std::string_view getMethod() const;
    virtual std::string_view getPath() const;
    virtual std::string_view getBody() const;
    virtual std::string_view getHeaders( const std::string& key) const;
    virtual std::string_view getHttpVersion() const;

    // Pure virtual method - must be implemented by derived classes
    // @return String identifying the message type ("Request" or "Response")
    virtual std::string getMessageType() const = 0;

  protected:
    std::string _method;  // HTTP method (GET, POST, DELETE)
    std::string _path;    // Request/response path
    std::string _body;    // Message body content
    std::string _httpVersion; //http version (HTTP/1.1 or HTTP/1.0)
    std::unordered_map<std::string, std::string> _headers; //content of Headers
};



