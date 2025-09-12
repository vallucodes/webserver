#pragma once

#include "../message/AMessage.hpp"
#include <algorithm>

class Request : public AMessage {
  private:
    bool isError;
    std::string _status;
  public:
    Request(void);
    ~Request(void);

    bool getError() const;
    void setError(bool val);

    std::string_view getStatus() const;
    void setStatus(const std::string& status);

    virtual std::string getMessageType() const override;
    void print() const;
};
