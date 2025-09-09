#pragma once

#include "../message/AMessage.hpp"


class Request : public AMessage {
  private:
  public:
  Request(void);
  ~Request(void);
  
  virtual std::string getMessageType() const override;
  void print() const;
};
