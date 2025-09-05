#include <gtest/gtest.h>
#include "../src/server/HelperFunctions.hpp"  // adjust path if needed

// 1
TEST(RequestCompleteTest, ReturnFalseForLargeBuffer) {
	bool status = true;
	std::string large_buffer(3 * 1024 * 1024, 'a'); // 3MB buffer

	EXPECT_FALSE(requestComplete(large_buffer, status));
	EXPECT_FALSE(status);
}

// 2
TEST(RequestCompleteTest, ReturnFalseIfNoHeaderEnd) {
	bool status = true;
	std::string buffer = "GET / HTTP/1.1\r\nContent-Length: 10"; // no \r\n\r\n header end

	EXPECT_FALSE(requestComplete(buffer, status));
	EXPECT_TRUE(status);
}

// 3
TEST(RequestCompleteTest, ReturnTrueForValidChunkedRequest) {
	bool status = true;
	std::string chunked_req =
		"GET / HTTP/1.1\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"4\r\nWiki\r\n"
		"0\r\n\r\n";

	EXPECT_TRUE(requestComplete(chunked_req, status));
	EXPECT_TRUE(status);
}

// 4
TEST(RequestCompleteTest, ReturnFalseNoBody) {
	bool status = true;
	std::string chunked_req =
		"GET / HTTP/1.1\r\n"
		"Transfer-Encoding: chunked\r\n"
		"4\r\nWiki\r\n"
		"0\r\n\r\n";

	EXPECT_FALSE(requestComplete(chunked_req, status));
	EXPECT_TRUE(status);
}

// 5
TEST(RequestCompleteTest, ReturnFalseForShortBody) {
	bool status = true;
	std::string buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length: 10\r\n"
		"\r\n"
		"12345";

	EXPECT_FALSE(requestComplete(buffer, status));
	EXPECT_TRUE(status);
}

// 6
TEST(RequestCompleteTest, ReturnTrueForCorrectBodySize) {
	bool status = true;
	std::string buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length: 5\r\n"
		"\r\n"
		"12345";

	EXPECT_TRUE(requestComplete(buffer, status));
	EXPECT_TRUE(status);
}

// 7
TEST(RequestCompleteTest, ReturnFalseForBigBodySize) {
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
TEST(RequestCompleteTest, ReturnFalseForEmptyRequest) {
	bool status = true;
	std::string buffer = "";

	EXPECT_FALSE(requestComplete(buffer, status));
	EXPECT_TRUE(status);
}

//9
TEST(RequestCompleteTest, ReturnFalseNoContentLengthBodyExists) {
	bool status = true;
	std::string buffer =
		"POST / HTTP/1.1\r\n"
		"\r\n"
		"12345678910";

	EXPECT_FALSE(requestComplete(buffer, status));
	EXPECT_FALSE(status);
}

//10
TEST(RequestCompleteTest, ReturnFalseCRLFCRLFInBodyStatusTrue) {
	bool status = true;
	std::string buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length: 500\r\n"
		"\r\n"
		"12345678910\r\n\r\n2134256";

	EXPECT_FALSE(requestComplete(buffer, status));
	EXPECT_TRUE(status);
}
// test for second \r\n\r\n in the body, should be valid
