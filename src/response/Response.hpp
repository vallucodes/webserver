#pragma once

class Response {
  public:
    Response();
    ~Response();

    void setStatus(std::string status);
    void setHeaders(std::string headers);
    void setBody(std::string body);

  private:
    std::string _status;
    std::string _headers;
    std::string _body;
};
