#include <gtest/gtest.h>
#include "../src/server/Cluster.hpp"  // adjust path if needed

#define MAX_BODY_SIZE 10000000

TEST(RequestCompleteTest, ReturnFalseForLargeBuffer) {

	std::string header =
    "POST /uploads HTTP/1.1\r\n"
    f"Host: {HOST}:{PORT}\r\n"
    "Transfer-Encoding: chunked\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n";

	std::string body = "";

	std::string buffer = header + body;

	decodeChunkedBody(buffer);

	
}
