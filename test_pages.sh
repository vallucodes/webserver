#!/bin/bash

# Test script for each page in www directory
# Tests all HTML files by making GET requests

echo "=========================================="
echo "Testing All Pages in www Directory"
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

# Get all HTML files in www directory
HTML_FILES=$(find www -name "*.html" -type f | sort)

# Counter for tests
TEST_COUNT=0
SUCCESS_COUNT=0

echo "Testing HTML pages..."
echo "===================="

for page in $HTML_FILES; do
    TEST_COUNT=$((TEST_COUNT + 1))

    # Remove 'www/' prefix to get the URL path
    url_path=$(echo $page | sed 's|^www/||')

    echo "$TEST_COUNT. Testing: /$url_path"
    echo "-------------------"

    # Make the request and capture both output and status
    response=$(curl -s --max-time 5 "$SERVER_URL/$url_path" -w "HTTP_STATUS:%{http_code}")
    status_code=$(echo "$response" | grep "HTTP_STATUS:" | cut -d':' -f2)
    content=$(echo "$response" | sed '/HTTP_STATUS:/d')

    echo "HTTP Status: $status_code"

    # Check if status is 200 (success)
    if [ "$status_code" = "200" ]; then
        echo "✓ SUCCESS"
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    else
        echo "✗ FAILED"
        echo "Response preview:"
        echo "$content" | head -3
    fi

    echo ""
done

echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo "Total pages tested: $TEST_COUNT"
echo "Successful requests: $SUCCESS_COUNT"
echo "Failed requests: $((TEST_COUNT - SUCCESS_COUNT))"
echo ""

if [ $SUCCESS_COUNT -eq $TEST_COUNT ]; then
    echo "🎉 All pages are accessible!"
else
    echo "⚠️  Some pages failed to load. Check server logs for details."
fi

echo "=========================================="
