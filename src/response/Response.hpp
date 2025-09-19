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

    /**
     * @brief Get HTTP status line
     * @return Status line string
     */
    std::string_view getStatus() const;

    /**
     * @brief Set HTTP status line
     * @param status Status line string
     */
    void setStatus(const std::string& status);

    /**
     * @brief Set HTTP header
     * @param key Header name
     * @param value Header value
     */
    virtual void setHeaders(const std::string& key, const std::string& value) override;

    /**
     * @brief Print response to console
     */
    void print() const;

  private:
    /**
     * @brief HTTP status line storage
     */
    std::string _status;

    /**
     * @brief HTTP headers storage
     */
    std::map<std::string, std::string> _headers;
};
