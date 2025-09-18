#!/bin/bash

# Individual handler testing script
# Tests each handler type separately with detailed output

echo "=========================================="
echo "Individual Handler Testing"
echo "=========================================="

# Detect server ports
PORTS=($(ss -tlnp | grep webserv | grep -o ':[0-9]*' | sed 's/://' | sort -n))
if [ ${#PORTS[@]} -eq 0 ]; then
    echo "Error: No webserver ports detected. Is the server running?"
    exit 1
fi
SERVER_URL="http://localhost:${PORTS[0]}"
echo "Using server URL: $SERVER_URL"
echo ""

# Test 1: GET Handler - Root page
echo "1. GET Handler - Root page"
echo "=========================="
curl -s --max-time 3 "$SERVER_URL/" -w "\nHTTP Status: %{http_code}\n"
echo ""

# Test 2: GET Handler - Static files
echo "2. GET Handler - Static files"
echo "============================="
echo "Testing index.html:"
curl -s --max-time 3 "$SERVER_URL/index.html" -w "\nHTTP Status: %{http_code}\n" | head -3
echo ""

echo "Testing image file:"
curl -s --max-time 3 "$SERVER_URL/imgs/lhaas.png" -w "\nHTTP Status: %{http_code}\n" | head -1
echo ""

# Test 3: GET Handler - Error case
echo "3. GET Handler - Error case (404)"
echo "================================="
curl -s --max-time 3 "$SERVER_URL/nonexistent.html" -w "\nHTTP Status: %{http_code}\n"
echo ""

# Test 4: POST Handler - File upload
echo "4. POST Handler - File upload"
echo "============================="
echo "Creating test file..."
echo "Test upload content" > /tmp/test_upload.txt
curl -s --max-time 5 -X POST "$SERVER_URL/uploads" -F "file=@/tmp/test_upload.txt" -w "\nHTTP Status: %{http_code}\n" | head -5
echo ""

# Test 5: DELETE Handler - File removal
echo "5. DELETE Handler - File removal"
echo "================================"
curl -s --max-time 3 -X DELETE "$SERVER_URL/uploads/test_upload.txt" -w "\nHTTP Status: %{http_code}\n" | head -5
echo ""

# Test 6: CGI Handler - Python script
echo "6. CGI Handler - Python script"
echo "=============================="
curl -s --max-time 5 "$SERVER_URL/cgi-bin/hello.py" -w "\nHTTP Status: %{http_code}\n" | head -10
echo ""

# Test 7: CGI Handler - JavaScript script
echo "7. CGI Handler - JavaScript script"
echo "=================================="
curl -s --max-time 5 "$SERVER_URL/cgi-bin/hello.js" -w "\nHTTP Status: %{http_code}\n" | head -10
echo ""

# Test 8: Redirect Handler
echo "8. Redirect Handler"
echo "==================="
echo "Testing permanent redirect:"
curl -s --max-time 3 -I "$SERVER_URL/old-page" -w "\nHTTP Status: %{http_code}\n"
echo ""

echo "Testing temporary redirect:"
curl -s --max-time 3 -I "$SERVER_URL/temp-redirect" -w "\nHTTP Status: %{http_code}\n"
echo ""

# Test 9: Method Not Allowed
echo "9. Method Not Allowed (405)"
echo "==========================="
curl -s --max-time 3 -X POST "$SERVER_URL/" -w "\nHTTP Status: %{http_code}\n"
echo ""

# Test 10: Large file upload (413)
echo "10. Large file upload (413)"
echo "==========================="
echo "Creating large file..."
dd if=/dev/zero of=/tmp/large_file.txt bs=1M count=2 2>/dev/null
curl -s --max-time 5 -X POST "$SERVER_URL/uploads" -F "file=@/tmp/large_file.txt" -w "\nHTTP Status: %{http_code}\n" | head -3
echo ""

# Cleanup
echo "Cleanup"
echo "======="
rm -f /tmp/test_upload.txt /tmp/large_file.txt
echo "Test files cleaned up."

echo ""
echo "=========================================="
echo "Testing Complete"
echo "=========================================="
