#!/bin/bash

# Improved test script for all HTTP handlers
# This script tests GET, POST, DELETE, CGI, and REDIRECT handlers using different ports

echo "=== Testing HTTP Handlers ==="
echo

# Function to test a handler
test_handler() {
    local config_file=$1
    local handler_name=$2
    local port=$3

    echo "Testing $handler_name handler with $config_file on port $port..."

    # Start server in background
    ./webserv $config_file &
    SERVER_PID=$!

    # Wait for server to start
    sleep 3

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

# Create port-specific configs
create_port_config() {
    local base_config=$1
    local port=$2
    local output_config=$3

    sed "s/listen 8080/listen $port/g" $base_config > $output_config
}

# Create port-specific configurations
create_port_config "configs/testGetHandler.conf" "8081" "configs/testGetHandler_8081.conf"
create_port_config "configs/testPostHandler.conf" "8082" "configs/testPostHandler_8082.conf"
create_port_config "configs/testDeleteHandler.conf" "8083" "configs/testDeleteHandler_8083.conf"
create_port_config "configs/testCgiHandler.conf" "8084" "configs/testCgiHandler_8084.conf"
create_port_config "configs/testRedirectHandler.conf" "8085" "configs/testRedirectHandler_8085.conf"

# Test each handler individually with different ports
test_handler "configs/testGetHandler_8081.conf" "GET" "8081"
test_handler "configs/testPostHandler_8082.conf" "POST" "8082"
test_handler "configs/testDeleteHandler_8083.conf" "DELETE" "8083"
test_handler "configs/testCgiHandler_8084.conf" "CGI" "8084"
test_handler "configs/testRedirectHandler_8085.conf" "REDIRECT" "8085"

# Clean up temporary configs
rm -f configs/test*_808*.conf

echo "=== All handler tests completed ==="
