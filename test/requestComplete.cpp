#include <gtest/gtest.h>
#include "../src/server/HelperFunctions.hpp"  // adjust path if needed

// 1
TEST(RequestCompleteTest, ReturnsFalseForLargeBuffer) {
	bool status = true;
	std::string large_buffer(3 * 1024 * 1024, 'a'); // 3MB buffer

	EXPECT_FALSE(requestComplete(large_buffer, status));
	EXPECT_FALSE(status);
}

// 2
TEST(RequestCompleteTest, ReturnsFalseIfNoHeaderEnd) {
	bool status = true;
	std::string buffer = "GET / HTTP/1.1\r\nContent-Length: 10"; // no \r\n\r\n header end

	EXPECT_FALSE(requestComplete(buffer, status));
}

// 3
TEST(RequestCompleteTest, ReturnsTrueForValidChunkedRequest) {
	bool status = true;
	std::string chunked_req =
		"GET / HTTP/1.1\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"4\r\nWiki\r\n"
		"0\r\n\r\n";

	EXPECT_TRUE(requestComplete(chunked_req, status));
}

// 4
TEST(RequestCompleteTest, ReturnsFalseNoBody) {
	bool status = true;
	std::string chunked_req =
		"GET / HTTP/1.1\r\n"
		"Transfer-Encoding: chunked\r\n"
		"4\r\nWiki\r\n"
		"0\r\n\r\n";

	EXPECT_FALSE(requestComplete(chunked_req, status));
}

// 5
TEST(RequestCompleteTest, ReturnsFalseForShortBody) {
	bool status = true;
	std::string buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length: 10\r\n"
		"\r\n"
		"12345";

	EXPECT_FALSE(requestComplete(buffer, status));
}

// 6
TEST(RequestCompleteTest, ReturnsTrueForCorrectBodySize) {
	bool status = true;
	std::string buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length: 5\r\n"
		"\r\n"
		"12345";

	EXPECT_TRUE(requestComplete(buffer, status));
}

// 7
TEST(RequestCompleteTest, ReturnsFalseForBigBodySize) {
	bool status = true;
	std::string buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length: 5\r\n"
		"\r\n"
		"12345678910";

	EXPECT_FALSE(requestComplete(buffer, status));
	EXPECT_FALSE(status);
}

// 8
TEST(RequestCompleteTest, ReturnsFalseForEmptyRequest) {
	bool status = true;
	std::string buffer = "";

	EXPECT_FALSE(requestComplete(buffer, status));
}
