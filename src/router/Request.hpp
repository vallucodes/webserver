#pragma once

#include <string>

class Request {
  public:
    Request();
    ~Request();

    std::string method() const;
    std::string path() const;
    std::string body() const;
    std::string status() const;

    void setMethod(const std::string& method);
    void setPath(const std::string& path);
    void setBody(const std::string& body);
    void setStatus(const std::string& status);

  private:
    std::string _method;
    std::string _path;
    std::string _body;
    std::string _status;
};
