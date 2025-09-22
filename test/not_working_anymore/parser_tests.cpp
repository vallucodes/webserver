#include <gtest/gtest.h>
#include "../src/parser/Parser.hpp"  // adjust path if needed
#include <sstream>
#include <iostream>

// Suppose parseRequest is in scope

TEST(RequestCompleteTest, PrintsCorrectOutput) {
    std::string buffer =
        "POST /submit-form HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "User-Agent: curl/7.85.0\r\n"
        "Accept: */*\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 27\r\n"
        "Cookie: session=abc123\r\n"
        "\r\n"
        "field1=value1&field2=value2";


    // Redirect cout to a stringstream
    std::stringstream ss;
    std::streambuf* oldCout = std::cout.rdbuf(ss.rdbuf());

    Request req = Parser::parseRequest(buffer);

    // Restore cout
    std::cout.rdbuf(oldCout);

    std::string output = ss.str();

    // Print actual captured output for debugging
    std::cerr << "=== Captured Output ===\n" << output << "\n====================\n";

    // Now check that the printed output contains expected text
    if (output.find("POST") == std::string::npos) {
    ADD_FAILURE() << "Expected 'POST' in output, but it was not found.";
    }
    if (output.find("/submit-form") == std::string::npos) {
        ADD_FAILURE() << "Expected '/submit-form' in output, but it was not found.";
    }
    if (output.find("HTTP/1.1") == std::string::npos) {
        ADD_FAILURE() << "Expected 'HTTP/1.1' in output, but it was not found.";
    }
    if (output.find("Host: www.example.com") == std::string::npos) {
        ADD_FAILURE() << "Expected 'Host: www.example.com' in output, but it was not found.";
    }
    if (output.find("User-Agent: curl/7.85.0") == std::string::npos) {
        ADD_FAILURE() << "Expected 'User-Agent: curl/7.85.0' in output, but it was not found.";
    }
    if (output.find("Accept: */*") == std::string::npos) {
        ADD_FAILURE() << "Expected 'Accept: */*' in output, but it was not found.";
    }
    if (output.find("Content-Type: application/x-www-form-urlencoded") == std::string::npos) {
        ADD_FAILURE() << "Expected 'Content-Type: application/x-www-form-urlencoded' in output, but it was not found.";
    }
    if (output.find("Content-Length: 27") == std::string::npos) {
        ADD_FAILURE() << "Expected 'Content-Length: 27' in output, but it was not found.";
    }
    if (output.find("Cookie: session=abc123") == std::string::npos) {
        ADD_FAILURE() << "Expected 'Cookie: session=abc123' in output, but it was not found.";
    }
    if (output.find("field1=value1&field2=value2") == std::string::npos) {
    ADD_FAILURE() << "Expected body in output, but it was not found.";
}


}

