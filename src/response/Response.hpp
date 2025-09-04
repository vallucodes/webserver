#pragma once

#include <string>
#include <iostream>

class Response {
  public:
    Response();
    ~Response();

    void setStatus(const std::string& status);
    void setBody(const std::string& body);

    const std::string& getStatus() const;
    const std::string& getBody() const;

  private:
    std::string _status;
    std::string _body;
};
