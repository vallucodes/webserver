#!/bin/bash

# WebServer Test Suite Runner
# This script runs all tests from TESTS.md one by one

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

# Function to print section headers
print_section() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}\n"
}

# Function to print test headers
print_test() {
    echo -e "\n${YELLOW}--- $1 ---${NC}"
}

# Function to run a test and check result
run_test() {
    local test_name="$1"
    local command="$2"
    local expected_code="$3"

    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    echo "Running: $test_name"
    echo "Command: $command"

    if [ -n "$expected_code" ]; then
        # Test with expected status code
        actual_code=$(eval "$command" -s -o /dev/null -w "%{http_code}" 2>/dev/null)
        if [ "$actual_code" = "$expected_code" ]; then
            echo -e "${GREEN}✓ PASS${NC} - Expected: $expected_code, Got: $actual_code"
            PASSED_TESTS=$((PASSED_TESTS + 1))
        else
            echo -e "${RED}✗ FAIL${NC} - Expected: $expected_code, Got: $actual_code"
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    else
        # Test without status code check (just run and show output)
        echo "Output:"
        eval "$command" 2>&1
        echo -e "${GREEN}✓ EXECUTED${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    fi
    echo ""
}

# Function to create test files
create_test_files() {
    echo "Creating test files..."
    echo "test content $(date)" > test.txt
    echo "delete me" > delete_test.txt
    echo "test content" > test2.txt
    echo "test content" > test3.txt
    echo "workflow test $(date)" > workflow_test.txt
    echo "test content" > test.txt
    echo "test content" > test2.txt
    echo "test content" > test3.txt
    echo "workflow test $(date)" > workflow_test.txt
}

# Function to cleanup test files
cleanup_test_files() {
    echo "Cleaning up test files..."
    rm -f test.txt test2.txt test3.txt delete_test.txt workflow_test.txt large_file.txt small_file.txt large_test.txt
    rm -f ./www/webserv_project/uploads/*

}

# Main execution
main() {
    echo -e "${GREEN}Starting WebServer Test Suite${NC}"
    echo "Make sure your webserver is running on ports 8080-8084"
    echo "Press Enter to continue or Ctrl+C to exit..."
    read -r

    create_test_files

    # 1. BASIC GET TESTS
    print_section "1. BASIC GET TESTS"

    print_test "Server 1 (Port 8080) - Root directory"
    run_test "Root directory" "curl -v http://127.0.0.1:8080/" "200"

    print_test "Server 1 (Port 8080) - Index file"
    run_test "Index file" "curl -v http://127.0.0.1:8080/index.html" "200"

    print_test "Server 1 (Port 8080) - Static files"
    run_test "Favicon" "curl -v http://127.0.0.1:8080/favicon.ico" "200"
    run_test "Upload page" "curl -v http://127.0.0.1:8080/upload.html" "200"
    run_test "Delete page" "curl -v http://127.0.0.1:8080/delete.html" "200"

    print_test "Server 1 (Port 8080) - Directory listings"
    run_test "Uploads directory" "curl -v http://127.0.0.1:8080/uploads/" "200"
    run_test "Images directory" "curl -v http://127.0.0.1:8080/imgs/" "200"
    run_test "Nested directory" "curl -v http://127.0.0.1:8080/nested/" "200"

    print_test "Server 1 (Port 8080) - CGI scripts"
    run_test "Python CGI" "curl -v http://127.0.0.1:8080/cgi-bin/hello.py" "200"
    run_test "JavaScript CGI" "curl -v http://127.0.0.1:8080/cgi-bin/hello.js" "200"

    print_test "Server 1 (Port 8080) - Redirection"
    run_test "Redirect (no follow)" "curl -v http://127.0.0.1:8080/old" "302"
    run_test "Redirect (follow)" "curl -v -L http://127.0.0.1:8080/old" "200"

    print_test "Server 1 (Port 8080) - Not found 404"
    run_test "CGI directory" "curl -v http://127.0.0.1:8080/cgi-bin/" "404"
    run_test "Non-existent file" "curl -v http://127.0.0.1:8080/noexist.html" "404"
    run_test "Non-existent directory" "curl -v http://127.0.0.1:8080/nodir/" "404"

    print_test "Multi-Server GET Tests"
    run_test "Server 1" "curl -v http://127.0.0.1:8080/" "200"
    run_test "Server 2" "curl -v http://127.0.0.1:8081/" "200"
    run_test "Server 3" "curl -v http://127.0.0.1:8082/" "200"
    run_test "Server 4" "curl -v http://127.0.0.1:8083/" "200"
    run_test "Server 5" "curl -v http://127.0.0.1:8084/" "200"

    # 1.5. NESTED AUTOINDEX TESTS
    print_section "1.5. NESTED AUTOINDEX TESTS"

    print_test "Server 1 (Port 8080) - Nested Directory Structure"
    run_test "Root nested directory" "curl -v http://127.0.0.1:8080/nested/" "200"
    run_test "Nested directory without slash" "curl -v http://127.0.0.1:8080/nested" "200"
    run_test "Deep nested directory" "curl -v http://127.0.0.1:8080/nested/deep/" "200"
    run_test "Deep nested directory without slash" "curl -v http://127.0.0.1:8080/nested/deep" "200"
    run_test "Deepest nested directory" "curl -v http://127.0.0.1:8080/nested/deep/folder/" "200"
    run_test "Deepest nested directory without slash" "curl -v http://127.0.0.1:8080/nested/deep/folder" "200"

    print_test "Server 1 (Port 8080) - Nested Directory Files"
    run_test "Nested HTML file" "curl -v http://127.0.0.1:8080/nested/nested.html" "200"
    run_test "Deep nested HTML file" "curl -v http://127.0.0.1:8080/nested/deep/folder/nested.html" "200"

    print_test "Server 1 (Port 8080) - Nested Directory Error Cases"
    run_test "Non-existent nested file" "curl -v http://127.0.0.1:8080/nested/nonexistent.html" "404"
    run_test "Non-existent deep nested file" "curl -v http://127.0.0.1:8080/nested/deep/nonexistent.html" "404"
    run_test "Non-existent deepest nested file" "curl -v http://127.0.0.1:8080/nested/deep/folder/nonexistent.html" "404"
    run_test "Non-existent nested directory" "curl -v http://127.0.0.1:8080/nested/nonexistent/" "404"
    run_test "Non-existent deep nested directory" "curl -v http://127.0.0.1:8080/nested/deep/nonexistent/" "404"

    # 2. POST/UPLOAD TESTS
    print_section "2. POST/UPLOAD TESTS"

    print_test "Server 1 (Port 8080) - File upload"
    run_test "File upload" "curl -v -X POST -F 'file=@test.txt' http://127.0.0.1:8080/uploads/" "201"
    run_test "Verify upload" "curl -v http://127.0.0.1:8080/uploads/test.txt" "200"

    print_test "Server 1 (Port 8080) - CGI POST"
    run_test "CGI POST form data" "curl -v -X POST -d 'name=test&value=123' http://127.0.0.1:8080/cgi-bin/hello.py" "200"
    run_test "CGI POST JSON" "curl -v -X POST -H 'Content-Type: application/json' -d '{\"name\":\"test\",\"value\":123}' http://127.0.0.1:8080/cgi-bin/hello.py" "200"

    print_test "Server 2 (Port 8081) - POST should be denied"
    run_test "POST denied" "curl -v -X POST -F 'file=@test2.txt' http://127.0.0.1:8081/uploads/" "405"

    print_test "Server 3 (Port 8082) - POST allowed"
    run_test "File upload" "curl -v -X POST -F 'file=@test3.txt' http://127.0.0.1:8082/uploads/" "201"
    run_test "Verify upload" "curl -v http://127.0.0.1:8082/uploads/test3.txt" "200"

    # 3. DELETE TESTS
    print_section "3. DELETE TESTS"

    print_test "Server 1 (Port 8080) - DELETE allowed"
    run_test "Upload file for deletion" "curl -v -X POST -F 'file=@delete_test.txt' http://127.0.0.1:8080/uploads/" "201"
    run_test "Delete file" "curl -v -X DELETE http://127.0.0.1:8080/uploads/delete_test.txt" "200"
    run_test "Delete non-existent file" "curl -v -X DELETE http://127.0.0.1:8080/uploads/nonexistent.txt" "404"

    print_test "Server 2 (Port 8081) - DELETE allowed"
    run_test "Delete non-existent file" "curl -v -X DELETE http://127.0.0.1:8081/uploads/nonexistent.txt" "404"

    print_test "Server 3 (Port 8082) - DELETE should be denied"
    run_test "DELETE denied" "curl -v -X DELETE http://127.0.0.1:8082/uploads/nonexistent.txt" "405"

    # 4. ERROR HANDLING TESTS
    print_section "4. ERROR HANDLING TESTS"

    print_test "404 Not Found"
    run_test "Non-existent file" "curl -v http://127.0.0.1:8080/nonexistent.html" "404"
    run_test "Non-existent CGI" "curl -v http://127.0.0.1:8080/cgi-bin/nonexistent.py" "404"
    run_test "Non-existent upload" "curl -v http://127.0.0.1:8080/uploads/nonexistent.txt" "404"

    print_test "405 Method Not Allowed"
    run_test "PUT method" "curl -v -X PUT http://127.0.0.1:8080/" "405"
    run_test "PATCH method" "curl -v -X PATCH http://127.0.0.1:8080/uploads/" "405"
    run_test "OPTIONS method" "curl -v -X OPTIONS http://127.0.0.1:8080/" "405"
    run_test "HEAD method" "curl -v -X HEAD http://127.0.0.1:8080/" "405"

    print_test "404 Not Found - POST to non-existent locations"
    run_test "POST to noupload" "curl -v -X POST -F 'file=@test.txt' http://127.0.0.1:8080/noupload/" "404"
    run_test "POST to nonexistent" "curl -v -X POST -F 'file=@test.txt' http://127.0.0.1:8080/nonexistent/" "404"

    print_test "404 Not Found - DELETE from non-existent locations"
    run_test "DELETE from noupload" "curl -v -X DELETE http://127.0.0.1:8080/noupload/test.txt" "404"
    run_test "DELETE from nonexistent" "curl -v -X DELETE http://127.0.0.1:8080/nonexistent/test.txt" "404"

    print_test "413 Payload Too Large"
    echo "Creating large file (2000KB)..."
    dd if=/dev/zero of=large_file.txt bs=1K count=2000 2>/dev/null
    run_test "Large file upload" "curl -v -X POST -F 'file=@large_file.txt' http://127.0.0.1:8080/uploads/" "413"

    print_test "File within size limit"
    echo "Creating small file (5KB)..."
    dd if=/dev/zero of=small_file.txt bs=1K count=5 2>/dev/null
    run_test "Small file upload" "curl -v -X POST -F 'file=@small_file.txt' http://127.0.0.1:8080/uploads/" "201"
    run_test "Delete small file" "curl -v -X DELETE http://127.0.0.1:8080/uploads/small_file.txt" "200"

    print_test "500 Server Error (CGI timeout)"
    run_test "CGI timeout" "curl -v http://127.0.0.1:8080/cgi-bin/inf.py" "504"

    # 5. STATUS CODE VERIFICATION
    print_section "5. STATUS CODE VERIFICATION"

    print_test "Quick Status Checks"
    run_test "Root status" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/" "200"
    run_test "Index status" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/index.html" "200"
    run_test "Favicon status" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/favicon.ico" "200"
    run_test "Uploads status" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/uploads/" "200"
    run_test "CGI status" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/cgi-bin/hello.py" "200"
    run_test "Nested directory status" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/nested/" "200"
    run_test "Deep nested directory status" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/nested/deep/" "200"
    run_test "Deepest nested directory status" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/nested/deep/folder/" "200"

    run_test "Redirect status" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/old" "302"
    run_test "Not found status" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/nonexistent.html" "404"

    run_test "PUT status" "curl -s -o /dev/null -w '%{http_code}' -X PUT http://127.0.0.1:8080/" "405"
    run_test "PATCH status" "curl -s -o /dev/null -w '%{http_code}' -X PATCH http://127.0.0.1:8080/uploads/" "405"

    run_test "POST to noupload status" "curl -s -o /dev/null -w '%{http_code}' -X POST -F 'file=@test.txt' http://127.0.0.1:8080/noupload/" "404"
    run_test "POST to cgi-bin status" "curl -s -o /dev/null -w '%{http_code}' -X POST -F 'file=@test.txt' http://127.0.0.1:8080/cgi-bin/" "404"

    echo "Creating large test file..."
    dd if=/dev/zero of=large_test.txt bs=1K count=2000 2>/dev/null
    run_test "Large file status" "curl -s -o /dev/null -w '%{http_code}' -X POST -F 'file=@large_test.txt' http://127.0.0.1:8080/uploads/" "413"

    print_test "Multi-server status check"
    run_test "Server 1 status" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/" "200"
    run_test "Server 2 status" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8081/" "200"
    run_test "Server 3 status" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8082/" "200"
    run_test "Server 4 status" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8083/" "200"
    run_test "Server 5 status" "curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8084/" "200"

    # 6. COMPLETE WORKFLOW TESTS
    print_section "6. COMPLETE WORKFLOW TESTS"

    print_test "Upload → Verify → Delete Workflow"
    run_test "Upload workflow file" "curl -v -X POST -F 'file=@workflow_test.txt' http://127.0.0.1:8080/uploads/" "201"
    run_test "Verify workflow file" "curl -v http://127.0.0.1:8080/uploads/workflow_test.txt" "200"
    run_test "Check directory listing" "curl -v http://127.0.0.1:8080/uploads/" "200"
    run_test "Delete workflow file" "curl -v -X DELETE http://127.0.0.1:8080/uploads/workflow_test.txt" "200"
    run_test "Verify file deleted" "curl -v http://127.0.0.1:8080/uploads/workflow_test.txt" "404"

    print_test "CGI Environment Test"
    run_test "CGI GET" "curl -v http://127.0.0.1:8080/cgi-bin/hello.py" "200"
    run_test "CGI POST form" "curl -v -X POST -d 'test=data' http://127.0.0.1:8080/cgi-bin/hello.py" "200"
    run_test "CGI POST JSON" "curl -v -X POST -H 'Content-Type: application/json' -d '{\"test\":\"json\"}' http://127.0.0.1:8080/cgi-bin/hello.py" "200"

    print_test "Nested Autoindex Navigation Test"
    run_test "Navigate to nested root" "curl -v http://127.0.0.1:8080/nested/" "200"
    run_test "Navigate to deep nested" "curl -v http://127.0.0.1:8080/nested/deep/" "200"
    run_test "Navigate to deepest nested" "curl -v http://127.0.0.1:8080/nested/deep/folder/" "200"
    run_test "Access nested HTML file" "curl -v http://127.0.0.1:8080/nested/nested.html" "200"
    run_test "Access deep nested HTML file" "curl -v http://127.0.0.1:8080/nested/deep/folder/nested.html" "200"

    # Cleanup
    cleanup_test_files

    # Final results
    print_section "TEST RESULTS SUMMARY"
    echo -e "Total Tests: ${TOTAL_TESTS}"
    echo -e "Passed: ${GREEN}${PASSED_TESTS}${NC}"
    echo -e "Failed: ${RED}${FAILED_TESTS}${NC}"

    if [ $FAILED_TESTS -eq 0 ]; then
        echo -e "\n${GREEN}🎉 ALL TESTS PASSED! 🎉${NC}"
        exit 0
    else
        echo -e "\n${RED}❌ SOME TESTS FAILED ❌${NC}"
        exit 1
    fi
}

# Run main function
main "$@"
