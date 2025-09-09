#pragma once

#include <string>
#include <iostream>
#include "../message/AMessage.hpp"
#include "../request/Request.hpp"

class Response: public AMessage {
  public:
    Response();
    ~Response();

    virtual std::string getMessageType() const override;

    std::string_view getStatus() const;

    void setStatus(const std::string& status);
  private:
    std::string _status;
};
