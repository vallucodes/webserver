#include <gtest/gtest.h>
#include "../src/server/HelperFunctions.hpp"
#include "../src/server/Cluster.hpp"

// Test 1: Simple chunked body decoding
TEST(DecodeChunkedBodyTest, DecodeSimpleChunkedBody) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer =
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

	client_state.data_validity = true;

	EXPECT_TRUE(decodeChunkedBody(client_state));
	EXPECT_EQ(client_state.clean_buffer, expected);
	EXPECT_TRUE(client_state.data_validity);
}

// Test 2: Single chunk
TEST(DecodeChunkedBodyTest, DecodeSingleChunk) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer =
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

	client_state.data_validity = true;
	EXPECT_TRUE(decodeChunkedBody(client_state));

	EXPECT_EQ(client_state.clean_buffer, expected);
	EXPECT_TRUE(client_state.data_validity);
}

// Test 3: Empty chunks (only terminator)
TEST(DecodeChunkedBodyTest, DecodeEmptyChunks) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer =
		"GET / HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"\r\n"
		"0\r\n\r\n";

	std::string expected =
		"GET / HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"\r\n";

	client_state.data_validity = true;
	decodeChunkedBody(client_state);

	EXPECT_EQ(client_state.clean_buffer, expected);
	EXPECT_TRUE(client_state.data_validity);
}

// Test 4: Hexadecimal chunk sizes
TEST(DecodeChunkedBodyTest, DecodeHexadecimalChunkSizes) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer =
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

	client_state.data_validity = true;
	decodeChunkedBody(client_state);

	EXPECT_EQ(client_state.clean_buffer, expected);
	EXPECT_TRUE(client_state.data_validity);
}

// Test 5: Multiple small chunks
TEST(DecodeChunkedBodyTest, DecodeMultipleSmallChunks) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer =
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

	client_state.data_validity = true;
	decodeChunkedBody(client_state);

	EXPECT_EQ(client_state.clean_buffer, expected);
	EXPECT_TRUE(client_state.data_validity);
}

// Test 6: Chunked body with binary data
TEST(DecodeChunkedBodyTest, DecodeChunkedBinaryData) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer =
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

	client_state.data_validity = true;
	decodeChunkedBody(client_state);

	EXPECT_EQ(client_state.clean_buffer, expected);
	EXPECT_TRUE(client_state.data_validity);
}

// Test 7: Malformed chunk (incomplete)
TEST(DecodeChunkedBodyTest, HandleIncompleteChunk) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer =
		"POST / HTTP/1.1\r\nContent-Type: text/plain\r\n\r\n5\r\nHello"; // Missing \r\n and terminator

	std::string expected =
		"POST / HTTP/1.1\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n";

	client_state.data_validity = true;
	EXPECT_FALSE(decodeChunkedBody(client_state));

	// EXPECT_EQ(client_state.clean_buffer, expected);
	EXPECT_FALSE(client_state.data_validity); // invalid chunk
}

// Test 8: Large chunk size in hex
TEST(DecodeChunkedBodyTest, DecodeLargeHexChunkSize) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	std::string large_data(255, 'A'); // 255 characters of 'A'
	client_state.buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"FF\r\n" + large_data + "\r\n"
		"0\r\n\r\n";

	std::string expected =
		"POST / HTTP/1.1\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n" + large_data;

	client_state.data_validity = true;
	decodeChunkedBody(client_state);

	EXPECT_EQ(client_state.clean_buffer, expected);
	EXPECT_TRUE(client_state.data_validity);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_BODY_SIZE 10000000

// Test 1: Body exceeds max_body_size should return false and set data_validity to false
TEST(IsRequestBodyCompleteTest, ReturnFalseForLargeBody) {
	ClientRequestState client_state;
	client_state.max_body_size = 1024;
	client_state.buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length: 2048\r\n"
		"\r\n"
		"body_content_here";
	size_t header_end = client_state.buffer.find("\r\n\r\n") + 4;

	EXPECT_FALSE(isRequestBodyComplete(client_state, header_end));
	EXPECT_FALSE(client_state.data_validity);
}

// Test 2: No Content-Length header should return true (no body expected)
TEST(IsRequestBodyCompleteTest, ReturnTrueIfNoContentLength) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer = "GET / HTTP/1.1\r\n\r\n";
	size_t header_end = client_state.buffer.find("\r\n\r\n") + 4;

	EXPECT_TRUE(isRequestBodyComplete(client_state, header_end));
	EXPECT_TRUE(client_state.data_validity);
	EXPECT_EQ(client_state.request_size, header_end);
}

// Test 3: Complete body with Content-Length should return true
TEST(IsRequestBodyCompleteTest, ReturnTrueForCompleteBody) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length: 4\r\n"
		"\r\n"
		"Wiki";
	size_t header_end = client_state.buffer.find("\r\n\r\n") + 4;

	EXPECT_TRUE(isRequestBodyComplete(client_state, header_end));
	EXPECT_TRUE(client_state.data_validity);
	EXPECT_EQ(client_state.request_size, header_end + 4);
}

// Test 4: Incomplete body should return false
TEST(IsRequestBodyCompleteTest, ReturnFalseForIncompleteBody) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length: 10\r\n"
		"\r\n"
		"Wiki";
	size_t header_end = client_state.buffer.find("\r\n\r\n") + 4;

	EXPECT_FALSE(isRequestBodyComplete(client_state, header_end));
	EXPECT_TRUE(client_state.data_validity);
}

// Test 5: Zero Content-Length should return true
TEST(IsRequestBodyCompleteTest, ReturnTrueForZeroContentLength) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length: 0\r\n"
		"\r\n";
	size_t header_end = client_state.buffer.find("\r\n\r\n") + 4;

	EXPECT_TRUE(isRequestBodyComplete(client_state, header_end));
	EXPECT_TRUE(client_state.data_validity);
	EXPECT_EQ(client_state.request_size, header_end);
}

// Test 6: Exact body size should return true
TEST(IsRequestBodyCompleteTest, ReturnTrueForExactBodySize) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length: 5\r\n"
		"\r\n"
		"12345";
	size_t header_end = client_state.buffer.find("\r\n\r\n") + 4;

	EXPECT_TRUE(isRequestBodyComplete(client_state, header_end));
	EXPECT_TRUE(client_state.data_validity);
	EXPECT_EQ(client_state.request_size, header_end + 5);
}

// Test 7: More data than Content-Length should return true and set correct request_size
TEST(IsRequestBodyCompleteTest, ReturnTrueWithExtraData) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	std::string original_buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length: 5\r\n"
		"\r\n"
		"12345EXTRA_DATA";
	client_state.buffer = original_buffer;
	size_t header_end = client_state.buffer.find("\r\n\r\n") + 4;

	EXPECT_TRUE(isRequestBodyComplete(client_state, header_end));
	EXPECT_TRUE(client_state.data_validity);
	EXPECT_EQ(client_state.request_size, header_end + 5);
	EXPECT_EQ(client_state.buffer, "EXTRA_DATA");
}

// Test 8: Empty buffer with header_end at 0 should return true
TEST(IsRequestBodyCompleteTest, ReturnTrueForEmptyBuffer) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer = "";
	size_t header_end = 0;

	EXPECT_TRUE(isRequestBodyComplete(client_state, header_end));
	EXPECT_TRUE(client_state.data_validity);
	EXPECT_EQ(client_state.request_size, 0);
}

// Test 9: Headers only without Content-Length should return true
TEST(IsRequestBodyCompleteTest, ReturnTrueForHeadersOnly) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer =
		"GET / HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"\r\n";
	size_t header_end = client_state.buffer.find("\r\n\r\n") + 4;

	EXPECT_TRUE(isRequestBodyComplete(client_state, header_end));
	EXPECT_TRUE(client_state.data_validity);
	EXPECT_EQ(client_state.request_size, header_end);
}

// Test 10: Content-Length with spaces should work
TEST(IsRequestBodyCompleteTest, ReturnTrueWithSpacesInContentLength) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length:   5   \r\n"  // Spaces around value
		"\r\n"
		"12345";
	size_t header_end = client_state.buffer.find("\r\n\r\n") + 4;

	EXPECT_TRUE(isRequestBodyComplete(client_state, header_end));
	EXPECT_TRUE(client_state.data_validity);
	EXPECT_EQ(client_state.request_size, header_end + 5);
}

// Test 11: Multiple Content-Length headers (first one should be used)
TEST(IsRequestBodyCompleteTest, ReturnTrueWithMultipleContentLength) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length: 5\r\n"
		"Content-Length: 10\r\n"  // Second one should be ignored
		"\r\n"
		"12345";
	size_t header_end = client_state.buffer.find("\r\n\r\n") + 4;

	EXPECT_TRUE(isRequestBodyComplete(client_state, header_end));
	EXPECT_TRUE(client_state.data_validity);
	EXPECT_EQ(client_state.request_size, header_end + 5); // Uses first Content-Length
}

// Test 12: Fake Content-Length in body should be ignored
TEST(IsRequestBodyCompleteTest, ReturnTrueIgnoreFakeContentLengthInBody) {
	ClientRequestState client_state;
	client_state.max_body_size = 1000;
	client_state.buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length: 30\r\n"
		"\r\n"
		"Content-Length: 500\r\nFakeBody1";
	size_t header_end = client_state.buffer.find("\r\n\r\n") + 4;

	EXPECT_TRUE(isRequestBodyComplete(client_state, header_end));
	EXPECT_TRUE(client_state.data_validity);
	EXPECT_EQ(client_state.request_size, header_end + 30);
}

// Test 13: Body at max_body_size limit should return true
TEST(IsRequestBodyCompleteTest, ReturnTrueForBodyAtMaxSize) {
	ClientRequestState client_state;
	client_state.max_body_size = 5;
	client_state.buffer =
		"POST / HTTP/1.1\r\n"
		"Content-Length: 5\r\n"
		"\r\n"
		"12345";
	size_t header_end = client_state.buffer.find("\r\n\r\n") + 4;

	EXPECT_TRUE(isRequestBodyComplete(client_state, header_end));
	EXPECT_TRUE(client_state.data_validity);
	EXPECT_EQ(client_state.request_size, header_end + 5);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(PopResponseChunk, ResponseSmallerThanMax) {
	ClientRequestState client;
	client.response = "Hello";

	std::string chunk = popResponseChunk(client);

	EXPECT_EQ(chunk, "Hello");
	EXPECT_TRUE(client.response.empty());
}

TEST(PopResponseChunk, ResponseEqualToMax) {
	ClientRequestState client;
	client.response = "12345678";

	std::string chunk = popResponseChunk(client);

	EXPECT_EQ(chunk, "12345678");
	EXPECT_TRUE(client.response.empty());
}

TEST(PopResponseChunk, ResponseLargerThanMax) {
	ClientRequestState client;
	client.response = "ABCDEFGHIJK";

	std::string chunk = popResponseChunk(client);

	EXPECT_EQ(chunk, "ABCDEFGH");
	EXPECT_EQ(client.response, "IJK");
}

TEST(PopResponseChunk, MultipleCallsConsumeAll) {
	ClientRequestState client;
	client.response = "ABCDEFGHIJKLMNOP";

	std::string first = popResponseChunk(client);
	std::string second = popResponseChunk(client);

	EXPECT_EQ(first, "ABCDEFGH");
	EXPECT_EQ(second, "IJKLMNOP");
	EXPECT_TRUE(client.response.empty());
}

TEST(PopResponseChunk, EmptyResponse) {
	ClientRequestState client;
	client.response = "";

	std::string chunk = popResponseChunk(client);

	EXPECT_EQ(chunk, "");
	EXPECT_TRUE(client.response.empty());
}
