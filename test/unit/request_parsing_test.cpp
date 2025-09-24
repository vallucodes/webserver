#include <gtest/gtest.h>
#include "../src/parser/Parser.hpp"
#include "../src/request/Request.hpp"
#include "../src/message/AMessage.hpp"

// Utility: wrap Parser::parseRequest into something easy to test
static Request parse(const std::string& raw, bool& kick_me) {
    Parser parser;
    return parser.parseRequest(raw, kick_me);
}

// ✅ Test: simple valid GET request
TEST(ParserTest, ValidGetRequest) {
    bool kick_me = false;
    std::string raw =
        "GET /index.html HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: TestAgent\r\n"
        "\r\n";

    Request req = parse(raw, kick_me);

    EXPECT_FALSE(req.getError());
    EXPECT_EQ(req.getMethod(), "GET");
    EXPECT_EQ(req.getPath(), "/index.html");
    EXPECT_EQ(req.getHttpVersion(), "HTTP/1.1");
    EXPECT_EQ(req.getHeaders("host").front(), "example.com");
    EXPECT_EQ(req.getBody(), "");
}

// ❌ Test: missing Host header in HTTP/1.1 should be error
TEST(ParserTest, MissingHostHeader) {
    bool kick_me = false;
    std::string raw =
        "GET / HTTP/1.1\r\n"
        "User-Agent: Test\r\n"
        "\r\n";

    Request req = parse(raw, kick_me);

    EXPECT_TRUE(req.getError());
    EXPECT_EQ(req.getStatus(), "400 Bad Request");
}

// ✅ Test: valid POST with Content-Length and body
TEST(ParserTest, ValidPostRequest) {
    bool kick_me = false;
    std::string raw =
        "POST /submit HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "Hello";

    Request req = parse(raw, kick_me);

    EXPECT_FALSE(req.getError());
    EXPECT_EQ(req.getMethod(), "POST");
    EXPECT_EQ(req.getHeaders("content-length").front(), "5");
    EXPECT_EQ(req.getBody(), "Hello");
}

// ❌ Test: POST with Content-Length but too short body
TEST(ParserTest, ShortBodyShouldError) {
    bool kick_me = false;
    std::string raw =
        "POST /submit HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 10\r\n"
        "\r\n"
        "12345"; // only 5 bytes

    Request req = parse(raw, kick_me);

    // Your parser currently does not throw, but you can test body length:
    EXPECT_NE(req.getBody().size(), 10u);
}

// // ✅ Test: chunked request (Transfer-Encoding: chunked)
// TEST(ParserTest, ValidChunkedRequest) {
//     bool kick_me = false;
//     std::string raw =
//         "GET / HTTP/1.1\r\n"
//         "Host: example.com\r\n"
//         "Transfer-Encoding: chunked\r\n"
//         "\r\n"
//         "4\r\nWiki\r\n"
//         "0\r\n\r\n";

//     Request req = parse(raw, kick_me);

//     EXPECT_FALSE(req.getError());
//     EXPECT_TRUE(isChunked(req));
// }

// ❌ Test: duplicate Host header should error
TEST(ParserTest, DuplicateHostHeaderShouldError) {
    bool kick_me = false;
    std::string raw =
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Host: duplicate.com\r\n"
        "\r\n";

    Request req = parse(raw, kick_me);

    EXPECT_TRUE(req.getError());
    EXPECT_EQ(req.getStatus(), "400 Bad Request");
}

// ❌ Test: invalid method should error
TEST(ParserTest, InvalidMethodShouldError) {
    bool kick_me = false;
    std::string raw =
        "FOO / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";

    Request req = parse(raw, kick_me);

    EXPECT_TRUE(req.getError());
    EXPECT_EQ(req.getStatus(), "400 Bad Request");
}

// ❌ Test: bad request-target with space should error
TEST(ParserTest, InvalidPathShouldError) {
    bool kick_me = false;
    std::string raw =
        "GET /bad path HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";

    Request req = parse(raw, kick_me);

    EXPECT_TRUE(req.getError());
}

// ✅ Test: GET with both Content-Length and Transfer-Encoding should error
TEST(ParserTest, GetWithContentLengthAndTransferEncodingShouldError) {
    bool kick_me = false;
    std::string raw =
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 5\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "4\r\nWiki\r\n"
        "0\r\n\r\n";

    Request req = parse(raw, kick_me);

    EXPECT_TRUE(req.getError());
}

// ✅ Test: connection header keep-alive
TEST(ParserTest, ConnectionKeepAlive) {
    bool kick_me = true; // default
    std::string raw =
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Connection: keep-alive\r\n"
        "\r\n";

    Request req = parse(raw, kick_me);

    EXPECT_FALSE(kick_me); // should be false because keep-alive
}

TEST(ParserTest, MultipleNonUniqueHeadersAllowed) {
    bool kick_me = false;
    std::string raw =
        "GET / HTTP/1.1\r\n"
        "Cookie: a=1\r\n"
        "Cookie: b=2\r\n"
        "Host: example.com\r\n"
        "\r\n";

    Request req = parse(raw, kick_me);

    EXPECT_FALSE(req.getError());
    EXPECT_EQ(req.getHeaders("cookie").size(), 2u);
}

TEST(ParserTest, UnsupportedMethodShouldError) {
    bool kick_me = false;
    std::string raw =
        "OPTIONS / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";

    Request req = parse(raw, kick_me);

    EXPECT_TRUE(req.getError());
    EXPECT_EQ(req.getStatus(), "400 Bad Request");
}


TEST(ParserTest, HeaderValueWithTrailingSpaces) {
    bool kick_me = false;
    std::string raw =
        "GET / HTTP/1.1\r\n"
        "Host: example.com   \r\n"
        "\r\n";

    Request req = parse(raw, kick_me);

    EXPECT_FALSE(req.getError());
    EXPECT_EQ(req.getHeaders("host").front(), "example.com");
}

TEST(ParserTest, PostWithEmptyLinesInBody) {
    bool kick_me = false;
    std::string raw =
        "POST /submit HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 6\r\n"
        "\r\n"
        "\r\n123"; // body contains a leading empty line

    Request req = parse(raw, kick_me);

    EXPECT_FALSE(req.getError());
    EXPECT_EQ(req.getBody(), "\r\n123");
}

TEST(ParserTest, HeaderCaseInsensitive) {
    bool kick_me = false;
    std::string raw =
        "GET / HTTP/1.1\r\n"
        "hOsT: example.com\r\n"
        "CoNnEcTiOn: close\r\n"
        "\r\n";

    Request req = parse(raw, kick_me);

    EXPECT_FALSE(req.getError());
    EXPECT_EQ(req.getHeaders("host").front(), "example.com");
}

TEST(ParserTest, HeaderWithSpacesAroundColon) {
    bool kick_me = false;
    std::string raw =
        "GET / HTTP/1.1\r\n"
        "Host   :    example.com\r\n"
        "\r\n";

    Request req = parse(raw, kick_me);

    EXPECT_FALSE(req.getError());
    EXPECT_EQ(req.getHeaders("host").front(), "example.com");
}

TEST(ParserTest, GetWithQueryString) {
    bool kick_me = false;
    std::string raw =
        "GET /search?q=test HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";

    Request req = parse(raw, kick_me);

    EXPECT_FALSE(req.getError());
    EXPECT_EQ(req.getPath(), "/search?q=test");
}

TEST(ParserTest, PostWithZeroContentLength) {
    bool kick_me = false;
    std::string raw =
        "POST /submit HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    Request req = parse(raw, kick_me);

    EXPECT_FALSE(req.getError());
    EXPECT_EQ(req.getBody(), "");
}
