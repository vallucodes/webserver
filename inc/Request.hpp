#pragma once

#include "AMessage.hpp"

class Request : public AMessage {
  public:
    Request(void);
    ~Request(void);

    virtual std::string getMessageType() const override;
};

Request createRequest(const std::string& httpString);
