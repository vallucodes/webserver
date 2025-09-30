#pragma once

#include <iostream>
#include <string_view>
#include <vector>
#include <unordered_map>

class AMessage {
  public:
    AMessage(void);
    virtual ~AMessage(void);

    virtual void setMethod(const std::string& method);
    virtual void setPath(const std::string& path);
    virtual void setBody(const std::string& body);
    virtual void setHeaders(const std::string& key, const std::string& value);
    virtual void setHttpVersion(const std::string& httpVersion);


    virtual std::string_view getMethod() const;
    virtual std::string_view getPath() const;
    virtual std::string_view getBody() const;
    const std::vector<std::string>& getHeaders(const std::string& key) const;
    const std::unordered_map<std::string, std::vector<std::string>>& getAllHeaders() const ;
    virtual std::string_view getHttpVersion() const;
    virtual std::string getMessageType() const = 0;

  protected:
    std::string _method;  
    std::string _path;    
    std::string _body;    
    std::string _httpVersion;
    std::unordered_map<std::string, std::vector<std::string>> _headers;
};
