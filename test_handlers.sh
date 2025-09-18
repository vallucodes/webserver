#!/bin/bash

# Comprehensive test script for webserver handlers
# This script tests all HTTP handlers: GET, POST, DELETE, CGI, and redirect

echo "=========================================="
echo "Webserver Handler Testing Suite"
echo "=========================================="

# Configuration
SERVER_URL="http://localhost:8080"
TEST_DIR="/tmp/webserver_test"
UPLOAD_FILE="$TEST_DIR/test_upload.txt"

# Create test directory and files
mkdir -p "$TEST_DIR"
echo "This is a test file for upload testing." > "$UPLOAD_FILE"

# Function to make HTTP requests and display results
test_request() {
    local method="$1"
    local path="$2"
    local description="$3"
    local data="$4"
    local headers="$5"

    echo ""
    echo "--- Testing: $description ---"
    echo "Method: $method | Path: $path"
    echo "----------------------------------------"

    if [ -n "$data" ]; then
        if [ -n "$headers" ]; then
            curl -s -X "$method" "$SERVER_URL$path" -d "$data" -H "$headers" -w "\nHTTP Status: %{http_code}\n" | head -20
        else
            curl -s -X "$method" "$SERVER_URL$path" -d "$data" -w "\nHTTP Status: %{http_code}\n" | head -20
        fi
    else
        if [ -n "$headers" ]; then
            curl -s -X "$method" "$SERVER_URL$path" -H "$headers" -w "\nHTTP Status: %{http_code}\n" | head -20
        else
            curl -s -X "$method" "$SERVER_URL$path" -w "\nHTTP Status: %{http_code}\n" | head -20
        fi
    fi
    echo ""
}

# Function to test file upload
test_file_upload() {
    local filename="$1"
    local description="$2"

    echo ""
    echo "--- Testing: $description ---"
    echo "Uploading file: $filename"
    echo "----------------------------------------"

    curl -s -X POST "$SERVER_URL/uploads" \
        -F "file=@$filename" \
        -w "\nHTTP Status: %{http_code}\n" | head -20
    echo ""
}

# Wait for server to start
echo "Waiting for server to start..."
sleep 2

# Test 1: GET Handler - Static Files
echo "=========================================="
echo "1. TESTING GET HANDLER - Static Files"
echo "=========================================="

test_request "GET" "/" "Root page (should serve index.html)"
test_request "GET" "/index.html" "Index page"
test_request "GET" "/upload.html" "Upload page"
test_request "GET" "/delete.html" "Delete page"
test_request "GET" "/imgs/lhaas.png" "Image file (PNG)"
test_request "GET" "/imgs/vlopatin.png" "Image file (PNG)"
test_request "GET" "/imgs/imunaev-.png" "Image file (PNG)"

# Test 2: GET Handler - Error Cases
echo "=========================================="
echo "2. TESTING GET HANDLER - Error Cases"
echo "=========================================="

test_request "GET" "/nonexistent.html" "Non-existent file (should return 404)"
test_request "GET" "/uploads/nonexistent.txt" "Non-existent upload file (should return 404)"

# Test 3: POST Handler - File Upload
echo "=========================================="
echo "3. TESTING POST HANDLER - File Upload"
echo "=========================================="

test_file_upload "$UPLOAD_FILE" "Upload text file"
test_file_upload "/dev/null" "Upload empty file"

# Test 4: POST Handler - Error Cases
echo "=========================================="
echo "4. TESTING POST HANDLER - Error Cases"
echo "=========================================="

test_request "POST" "/uploads" "POST without file data" "test=data"
test_request "POST" "/uploads" "POST with invalid content type" "" "Content-Type: application/json"

# Test 5: DELETE Handler - File Removal
echo "=========================================="
echo "5. TESTING DELETE HANDLER - File Removal"
echo "=========================================="

# First upload a file to delete
test_file_upload "$UPLOAD_FILE" "Upload file for deletion test"
test_request "DELETE" "/uploads/test_upload.txt" "Delete uploaded file"
test_request "DELETE" "/uploads/nonexistent.txt" "Delete non-existent file (should return 404)"
test_request "DELETE" "/uploads/../index.html" "Delete file outside uploads (should return 400)"

# Test 6: CGI Handler - Python Scripts
echo "=========================================="
echo "6. TESTING CGI HANDLER - Python Scripts"
echo "=========================================="

test_request "GET" "/cgi-bin/hello.py" "Python CGI script (GET)"
test_request "POST" "/cgi-bin/hello.py" "Python CGI script (POST)" "test=post_data" "Content-Type: application/x-www-form-urlencoded"

# Test 7: CGI Handler - JavaScript Scripts
echo "=========================================="
echo "7. TESTING CGI HANDLER - JavaScript Scripts"
echo "=========================================="

test_request "GET" "/cgi-bin/hello.js" "JavaScript CGI script (GET)"
test_request "POST" "/cgi-bin/hello.js" "JavaScript CGI script (POST)" "test=post_data" "Content-Type: application/x-www-form-urlencoded"

# Test 8: CGI Handler - Error Cases
echo "=========================================="
echo "8. TESTING CGI HANDLER - Error Cases"
echo "=========================================="

test_request "GET" "/cgi-bin/nonexistent.py" "Non-existent CGI script (should return 404)"
test_request "GET" "/cgi-bin/hello.c" "C script (not supported, should return 404)"

# Test 9: Redirect Handler
echo "=========================================="
echo "9. TESTING REDIRECT HANDLER"
echo "=========================================="

test_request "GET" "/old-page" "Permanent redirect (301)"
test_request "GET" "/temp-redirect" "Temporary redirect (302)"
test_request "GET" "/redirect-home" "Redirect to home (302)"

# Test 10: Method Not Allowed
echo "=========================================="
echo "10. TESTING METHOD NOT ALLOWED"
echo "=========================================="

test_request "POST" "/" "POST to root (should return 405)"
test_request "DELETE" "/" "DELETE to root (should return 405)"
test_request "PUT" "/index.html" "PUT to static file (should return 405)"

# Test 11: Large File Upload (Payload Too Large)
echo "=========================================="
echo "11. TESTING LARGE FILE UPLOAD"
echo "=========================================="

# Create a large file (>1MB)
dd if=/dev/zero of="$TEST_DIR/large_file.txt" bs=1M count=2 2>/dev/null
test_file_upload "$TEST_DIR/large_file.txt" "Large file upload (should return 413)"

# Test 12: Directory Listing
echo "=========================================="
echo "12. TESTING DIRECTORY ACCESS"
echo "=========================================="

test_request "GET" "/uploads/" "Uploads directory (should serve default file or 404)"
test_request "GET" "/www/" "WWW directory (should serve default file or 404)"

# Cleanup
echo "=========================================="
echo "CLEANUP"
echo "=========================================="
rm -rf "$TEST_DIR"
echo "Test files cleaned up."

echo ""
echo "=========================================="
echo "TESTING COMPLETE"
echo "=========================================="
echo "All handler tests have been executed."
echo "Check the output above for any errors or unexpected behavior."
