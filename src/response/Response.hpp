/**
 * @file Response.hpp
 * @brief HTTP Response class
 */

#pragma once

#include <string>
#include <iostream>
#include <map>
#include "../message/AMessage.hpp"

/**
 * @class Response
 * @brief HTTP response message
 */
class Response: public AMessage {
  public:
    Response();
    ~Response();

    /** Get message type identifier */
    virtual std::string getMessageType() const override;

    /** Get HTTP status line */
    std::string_view getStatus() const;

    /** Set HTTP status line */
    void setStatus(const std::string& status);

    /** Set HTTP header */
    virtual void setHeaders(const std::string& key, const std::string& value) override;

    /** Print response to console for debugging */
    void print() const;

  private:
    std::string _status;
    std::map<std::string, std::string> _headers;
};
