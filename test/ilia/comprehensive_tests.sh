#!/bin/bash

# Comprehensive WebServer Tests
# Based on configs/ilia/default.conf configuration
# Tests all 5 servers with their specific capabilities

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Test result tracking
test_results=()

# Function to run a test and track results
run_test() {
    local test_name="$1"
    local command="$2"
    local expected_status="$3"
    local description="$4"

    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    echo -e "${BLUE}Testing: $test_name${NC}"
    echo -e "Description: $description"
    echo -e "Command: $command"

    # Run the test and capture status code
    local actual_status
    if [[ "$expected_status" == "ignore" ]]; then
        # For tests where we don't care about status code
        eval "$command" > /dev/null 2>&1
        actual_status=$?
    else
        # For tests where we check status code
        actual_status=$(eval "$command" 2>/dev/null | tail -c 4)
    fi

    # Check if test passed
    if [[ "$expected_status" == "ignore" ]] || [[ "$actual_status" == "$expected_status" ]]; then
        echo -e "${GREEN}✓ PASSED${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        test_results+=("PASS: $test_name")
    else
        echo -e "${RED}✗ FAILED (Expected: $expected_status, Got: $actual_status)${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        test_results+=("FAIL: $test_name (Expected: $expected_status, Got: $actual_status)")
    fi
    echo ""
}

# Function to run a test with content check
run_content_test() {
    local test_name="$1"
    local command="$2"
    local expected_content="$3"
    local description="$4"

    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    echo -e "${BLUE}Testing: $test_name${NC}"
    echo -e "Description: $description"
    echo -e "Command: $command"

    # Run the test and capture content
    local actual_content
    actual_content=$(eval "$command" 2>/dev/null)

    # Check if expected content is found
    if echo "$actual_content" | grep -q "$expected_content"; then
        echo -e "${GREEN}✓ PASSED${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        test_results+=("PASS: $test_name")
    else
        echo -e "${RED}✗ FAILED (Expected content not found: $expected_content)${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        test_results+=("FAIL: $test_name (Expected content not found: $expected_content)")
    fi
    echo ""
}

# Print test summary
print_summary() {
    echo -e "${YELLOW}========================================${NC}"
    echo -e "${YELLOW}TEST SUMMARY${NC}"
    echo -e "${YELLOW}========================================${NC}"
    echo -e "Total Tests: $TOTAL_TESTS"
    echo -e "${GREEN}Passed: $PASSED_TESTS${NC}"
    echo -e "${RED}Failed: $FAILED_TESTS${NC}"
    echo ""

    if [ $FAILED_TESTS -gt 0 ]; then
        echo -e "${RED}FAILED TESTS:${NC}"
        for result in "${test_results[@]}"; do
            if [[ $result == FAIL:* ]]; then
                echo -e "${RED}$result${NC}"
            fi
        done
    fi

    echo -e "${YELLOW}========================================${NC}"
}

echo -e "${YELLOW}Starting Comprehensive WebServer Tests${NC}"
echo -e "${YELLOW}Based on configs/ilia/default.conf${NC}"
echo ""

# =============================================================================
# SERVER 1 (PORT 8080) - FULL FUNCTIONALITY TESTS
# =============================================================================
echo -e "${YELLOW}=== SERVER 1 (PORT 8080) - FULL FUNCTIONALITY ===${NC}"

# Basic GET requests
run_test "Server1-Root" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/" "200" "GET root directory"
run_test "Server1-Index" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/index.html" "200" "GET index.html"
run_test "Server1-Favicon" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/favicon.ico" "200" "GET favicon.ico"
run_test "Server1-Uploads" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/uploads/" "200" "GET uploads directory listing"
run_test "Server1-Images" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/imgs/" "200" "GET images directory listing"
run_test "Server1-CGI-Bin" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/cgi-bin/" "200" "GET CGI bin directory listing"

# Redirection test
run_test "Server1-Redirect" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/old" "302" "GET redirect from /old"

# CGI script tests
run_test "Server1-CGI-Python" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/cgi-bin/hello.py" "200" "GET CGI Python script"
run_test "Server1-CGI-JavaScript" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/cgi-bin/hello.js" "200" "GET CGI JavaScript script"

# POST tests (file upload)
echo "Creating test file for upload..."
echo "test content $(date)" > /tmp/upload_test.txt
run_test "Server1-Upload" "curl -s -o /dev/null -w '%{http_code}' -X POST -F 'file=@/tmp/upload_test.txt' http://127.0.0.1:8080/uploads/" "201" "POST file upload"
run_test "Server1-Upload-Verify" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/uploads/upload_test.txt" "200" "GET uploaded file"

# DELETE tests
run_test "Server1-Delete" "curl -s -o /dev/null -w '%{http_code}' -X DELETE http://127.0.0.1:8080/uploads/upload_test.txt" "204" "DELETE uploaded file"
run_test "Server1-Delete-Nonexistent" "curl -s -o /dev/null -w '%{http_code}' -X DELETE http://127.0.0.1:8080/uploads/nonexistent.txt" "404" "DELETE nonexistent file"

# Method not allowed tests
run_test "Server1-Method-PUT" "curl -s -o /dev/null -w '%{http_code}' -X PUT http://127.0.0.1:8080/" "405" "PUT method not allowed on root"
run_test "Server1-Method-PATCH" "curl -s -o /dev/null -w '%{http_code}' -X PATCH http://127.0.0.1:8080/uploads/" "405" "PATCH method not allowed on uploads"

# Not found tests
run_test "Server1-NotFound" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/nonexistent.html" "404" "GET nonexistent file"

# Cleanup
rm -f /tmp/upload_test.txt

# =============================================================================
# SERVER 2 (PORT 8081) - GET AND DELETE ONLY (NO POST)
# =============================================================================
echo -e "${YELLOW}=== SERVER 2 (PORT 8081) - GET AND DELETE ONLY ===${NC}"

# Basic GET requests
run_test "Server2-Root" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8081/" "200" "GET root directory"

# Upload test (should fail - POST not allowed)
echo "Creating test file for upload..."
echo "test content $(date)" > /tmp/upload_test2.txt
run_test "Server2-Upload-Denied" "curl -s -o /dev/null -w '%{http_code}' -X POST -F 'file=@/tmp/upload_test2.txt' http://127.0.0.1:8081/uploads/" "405" "POST file upload should be denied"

# DELETE should work (but file doesn't exist)
run_test "Server2-Delete-Nonexistent" "curl -s -o /dev/null -w '%{http_code}' -X DELETE http://127.0.0.1:8081/uploads/nonexistent.txt" "404" "DELETE nonexistent file"

# Cleanup
rm -f /tmp/upload_test2.txt

# =============================================================================
# SERVER 3 (PORT 8082) - GET AND POST ONLY (NO DELETE)
# =============================================================================
echo -e "${YELLOW}=== SERVER 3 (PORT 8082) - GET AND POST ONLY ===${NC}"

# Basic GET requests
run_test "Server3-Root" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8082/" "200" "GET root directory"

# Upload test (should work)
echo "Creating test file for upload..."
echo "test content $(date)" > /tmp/upload_test3.txt
run_test "Server3-Upload" "curl -s -o /dev/null -w '%{http_code}' -X POST -F 'file=@/tmp/upload_test3.txt' http://127.0.0.1:8082/uploads/" "201" "POST file upload should work"

# DELETE should be denied
run_test "Server3-Delete-Denied" "curl -s -o /dev/null -w '%{http_code}' -X DELETE http://127.0.0.1:8082/uploads/upload_test3.txt" "405" "DELETE should be denied"

# Cleanup
rm -f /tmp/upload_test3.txt

# =============================================================================
# SERVER 4 (PORT 8083) - SITE2 SIMPLE
# =============================================================================
echo -e "${YELLOW}=== SERVER 4 (PORT 8083) - SITE2 SIMPLE ===${NC}"

# Basic GET requests
run_test "Server4-Root" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8083/" "200" "GET site2 root"
run_test "Server4-Favicon" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8083/favicon.ico" "200" "GET site2 favicon"

# =============================================================================
# SERVER 5 (PORT 8084) - SITE3 SIMPLE
# =============================================================================
echo -e "${YELLOW}=== SERVER 5 (PORT 8084) - SITE3 SIMPLE ===${NC}"

# Basic GET requests
run_test "Server5-Root" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8084/" "200" "GET site3 root"
run_test "Server5-Favicon" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8084/favicon.ico" "200" "GET site3 favicon"

# =============================================================================
# ADVANCED TESTS
# =============================================================================
echo -e "${YELLOW}=== ADVANCED TESTS ===${NC}"

# Large file upload test (testing client_max_body_size)
echo "Creating large test file..."
dd if=/dev/zero of=/tmp/large_test.txt bs=1M count=5 2>/dev/null
run_test "Server1-Large-Upload" "curl -s -o /dev/null -w '%{http_code}' -X POST -F 'file=@/tmp/large_test.txt' http://127.0.0.1:8080/uploads/" "200" "Large file upload (5MB)"
rm -f /tmp/large_test.txt

# CGI POST with data
run_test "Server1-CGI-POST" "curl -s -o /dev/null -w '%{http_code}' -X POST -d 'name=test&value=123' http://127.0.0.1:8080/cgi-bin/hello.py" "200" "CGI POST with form data"

# Multiple clients test (simulate concurrent requests)
run_test "Server1-Concurrent-1" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/" "200" "Concurrent request 1"
run_test "Server1-Concurrent-2" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/" "200" "Concurrent request 2"
run_test "Server1-Concurrent-3" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/" "200" "Concurrent request 3"

# Print final summary
print_summary

# Exit with appropriate code
if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}All tests passed! 🎉${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed. Please check the output above.${NC}"
    exit 1
fi
