#!/bin/bash

# Test script for all HTTP handlers
# This script tests GET, POST, DELETE, CGI, and REDIRECT handlers

echo "=== Testing HTTP Handlers ==="
echo

# Function to test a handler
test_handler() {
    local config_file=$1
    local handler_name=$2
    local port=$3

    echo "Testing $handler_name handler with $config_file..."

    # Start server in background
    ./webserv $config_file &
    SERVER_PID=$!

    # Wait for server to start
    sleep 2

    # Test the handler
    case $handler_name in
        "GET")
            echo "  - Testing GET / (should return index.html or autoindex)"
            curl -s -o /dev/null -w "HTTP Status: %{http_code}\n" http://localhost:$port/

            echo "  - Testing GET /testdir/ (should return autoindex)"
            curl -s -o /dev/null -w "HTTP Status: %{http_code}\n" http://localhost:$port/testdir/

            echo "  - Testing GET /nonexistent (should return 404)"
            curl -s -o /dev/null -w "HTTP Status: %{http_code}\n" http://localhost:$port/nonexistent
            ;;

        "POST")
            echo "  - Testing POST /upload (file upload)"
            echo "test content" > /tmp/test_upload.txt
            curl -s -o /dev/null -w "HTTP Status: %{http_code}\n" -X POST -F "file=@/tmp/test_upload.txt" http://localhost:$port/upload
            rm -f /tmp/test_upload.txt
            ;;

        "DELETE")
            echo "  - Testing DELETE /uploads/h.png (file deletion)"
            curl -s -o /dev/null -w "HTTP Status: %{http_code}\n" -X DELETE http://localhost:$port/uploads/h.png
            ;;

        "CGI")
            echo "  - Testing GET /cgi-bin/hello.py (CGI script)"
            curl -s -o /dev/null -w "HTTP Status: %{http_code}\n" http://localhost:$port/cgi-bin/hello.py

            echo "  - Testing GET /cgi-bin/hello.js (CGI script)"
            curl -s -o /dev/null -w "HTTP Status: %{http_code}\n" http://localhost:$port/cgi-bin/hello.js
            ;;

        "REDIRECT")
            echo "  - Testing GET /redirect (should redirect to /index.html)"
            curl -s -o /dev/null -w "HTTP Status: %{http_code}\n" -L http://localhost:$port/redirect
            ;;
    esac

    # Stop server
    kill $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null

    echo "  $handler_name handler test completed."
    echo
}

# Test each handler individually
test_handler "configs/testGetHandler.conf" "GET" "8080"
test_handler "configs/testPostHandler.conf" "POST" "8080"
test_handler "configs/testDeleteHandler.conf" "DELETE" "8080"
test_handler "configs/testCgiHandler.conf" "CGI" "8080"
test_handler "configs/testRedirectHandler.conf" "REDIRECT" "8080"

echo "=== All handler tests completed ==="
