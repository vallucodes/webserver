#include <gtest/gtest.h>
#include "../src/server/HelperFunctions.hpp"
#include "../src/server/Cluster.hpp"  // adjust path if needed

#define MAX_BODY_SIZE 10000000

// Test 1: Simple chunked body decoding
TEST(DecodeChunkedBodyTest, DecodeSimpleChunkedBody) {
	std::string buffer =
		"POST / HTTP/1.1\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"4\r\nWiki\r\n"
		"5\r\npedia\r\n"
		"0\r\n\r\n";

	std::string expected =
		"POST / HTTP/1.1\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"Wikipedia";

	bool data_validity = true;
	decodeChunkedBody(buffer, data_validity);

	EXPECT_EQ(buffer, expected);
	EXPECT_TRUE(data_validity);
}

// Test 2: Single chunk
TEST(DecodeChunkedBodyTest, DecodeSingleChunk) {
	std::string buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"B\r\nHello World\r\n"
		"0\r\n\r\n";

	std::string expected =
		"POST / HTTP/1.1\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"Hello World";

	bool data_validity = true;
	decodeChunkedBody(buffer, data_validity);

	EXPECT_EQ(buffer, expected);
	EXPECT_TRUE(data_validity);
}

// Test 3: Empty chunks (only terminator)
TEST(DecodeChunkedBodyTest, DecodeEmptyChunks) {
	std::string buffer =
		"GET / HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"\r\n"
		"0\r\n\r\n";

	std::string expected =
		"GET / HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"\r\n";

	bool data_validity = true;
	decodeChunkedBody(buffer, data_validity);

	EXPECT_EQ(buffer, expected);
	EXPECT_TRUE(data_validity);
}

// Test 4: Hexadecimal chunk sizes
TEST(DecodeChunkedBodyTest, DecodeHexadecimalChunkSizes) {
	std::string buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Type: application/json\r\n"
		"\r\n"
		"F\r\n{\"test\":\"data\"}\r\n"
		"B\r\n, \"more\":1}\r\n"
		"0\r\n\r\n";

	std::string expected =
		"POST / HTTP/1.1\r\n"
		"Content-Type: application/json\r\n"
		"\r\n"
		"{\"test\":\"data\"}, \"more\":1}";

	bool data_validity = true;
	decodeChunkedBody(buffer, data_validity);

	EXPECT_EQ(buffer, expected);
	EXPECT_TRUE(data_validity);
}

// Test 5: Multiple small chunks
TEST(DecodeChunkedBodyTest, DecodeMultipleSmallChunks) {
	std::string buffer =
		"POST /upload HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"\r\n"
		"1\r\nH\r\n"
		"1\r\ne\r\n"
		"1\r\nl\r\n"
		"1\r\nl\r\n"
		"1\r\no\r\n"
		"0\r\n\r\n";

	std::string expected =
		"POST /upload HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"\r\n"
		"Hello";

	bool data_validity = true;
	decodeChunkedBody(buffer, data_validity);

	EXPECT_EQ(buffer, expected);
	EXPECT_TRUE(data_validity);
}

// Test 6: Chunked body with binary data
TEST(DecodeChunkedBodyTest, DecodeChunkedBinaryData) {
	std::string buffer =
		"POST /binary HTTP/1.1\r\n"
		"Content-Type: application/octet-stream\r\n"
		"\r\n"
		"3\r\n\x01\x02\x03\r\n"
		"2\r\n\xFF\xFE\r\n"
		"0\r\n\r\n";

	std::string expected =
		"POST /binary HTTP/1.1\r\n"
		"Content-Type: application/octet-stream\r\n"
		"\r\n"
		"\x01\x02\x03\xFF\xFE";

	bool data_validity = true;
	decodeChunkedBody(buffer, data_validity);

	EXPECT_EQ(buffer, expected);
	EXPECT_TRUE(data_validity);
}

// Test 7: Malformed chunk (incomplete)
TEST(DecodeChunkedBodyTest, HandleIncompleteChunk) {
	std::string buffer =
		"POST / HTTP/1.1\r\nContent-Type: text/plain\r\n\r\n5\r\nHello"; // Missing \r\n and terminator

	std::string expected =
		"POST / HTTP/1.1\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n";

	bool data_validity = true;
	decodeChunkedBody(buffer, data_validity);

	// EXPECT_EQ(buffer, expected);
	EXPECT_FALSE(data_validity); // invalid chunk
}

// Test 8: Large chunk size in hex
TEST(DecodeChunkedBodyTest, DecodeLargeHexChunkSize) {
	std::string large_data(255, 'A'); // 255 characters of 'A'
	std::string buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"FF\r\n" + large_data + "\r\n"
		"0\r\n\r\n";

	std::string expected =
		"POST / HTTP/1.1\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n" + large_data;

	bool data_validity = true;
	decodeChunkedBody(buffer, data_validity);

	EXPECT_EQ(buffer, expected);
	EXPECT_TRUE(data_validity);
}
