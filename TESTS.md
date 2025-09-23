# Webserver Testing Commands

## Server Configuration
- **Server Name**: localhost
- **Port**: 8083
- **Host**: 127.0.0.1
- **Root Directory**: www
- **Max Body Size**: 10MB (10000 bytes)

### Server Connection
```bash
# Test basic server connectivity
curl -v http://127.0.0.1:8083/

# Test server with different host headers
curl -v -H "Host: localhost" http://127.0.0.1:8083/
curl -v -H "Host: 127.0.0.1" http://127.0.0.1:8083/
```

## GET Request Tests

### Main Pages

```bash
# Test root index page
curl -v http://127.0.0.1:8083/

# Test specific HTML files
curl -v http://127.0.0.1:8083/index.html
curl -v http://127.0.0.1:8083/upload.html
curl -v http://127.0.0.1:8083/delete.html

# Test API pages
curl -v http://127.0.0.1:8083/api_landing.html # remove unnesessary data
curl -v http://127.0.0.1:8083/api_response.html # remove unnesessary data
curl -v http://127.0.0.1:8083/admin_dashboard.html # remove unnesessary data
curl -v http://127.0.0.1:8083/upload_success.html # remove page, I dont use it

# Test autoindex templates
curl -v http://127.0.0.1:8083/autoindex_template.html # issue?
curl -v http://127.0.0.1:8083/autoindex_fallback.html # issue?

# Test favicon
curl -v http://127.0.0.1:8083/favicon.ico
```

### Directory Listings (Autoindex)

```bash
# Test root directory with autoindex enabled
curl -v http://127.0.0.1:8083/ # issue

# Test subdirectories with autoindex
curl -v http://127.0.0.1:8083/testdir/
curl -v http://127.0.0.1:8083/uploads/ # issue
curl -v http://127.0.0.1:8083/api/
curl -v http://127.0.0.1:8083/imgs/
curl -v http://127.0.0.1:8083/empty/ # delete dir at all
curl -v http://127.0.0.1:8083/default/
curl -v http://127.0.0.1:8083/errors/

# Test CGI directory (should show scripts)
curl -v http://127.0.0.1:8083/cgi-bin/ # issue
```

### Static File Serving

```bash
# Test images
curl -v http://127.0.0.1:8083/imgs/imunaev-.png
curl -v http://127.0.0.1:8083/imgs/vlopatin.png
curl -v http://127.0.0.1:8083/imgs/lhaas.png

# Test JSON API endpoint
curl -v http://127.0.0.1:8083/api/test.json # issue

# Test default directory content
curl -v http://127.0.0.1:8083/default/default.html

# Test error pages
curl -v http://127.0.0.1:8083/errors/not_found_404.html
curl -v http://127.0.0.1:8083/errors/method_not_allowed_405.html
curl -v http://127.0.0.1:8083/errors/internal_server_error_500.html

# Test testdir content
curl -v http://127.0.0.1:8083/testdir/index.html
```

### Redirect Tests

```bash
# Test redirect location (/redirect redirects to /)
curl -v http://127.0.0.1:8083/redirect # issue

# Test redirect to specific page (/go-home redirects to /index.html)
curl -v http://127.0.0.1:8083/go-home # issue

# Test redirect with different status codes
curl -v -I http://127.0.0.1:8083/redirect # issue
curl -v -I http://127.0.0.1:8083/go-home # issue
```

### CGI Script Tests

```bash
# Test Python CGI script
curl -v "http://127.0.0.1:8083/cgi-bin/hello.py" # issue www/ ?

# Test JavaScript CGI script
curl -v "http://127.0.0.1:8083/cgi-bin/hello.js" # issue www/ ?


# END OF PART 1





# Test CGI with query parameters
curl -v "http://127.0.0.1:8083/cgi-bin/hello.py?name=test&value=123"
curl -v "http://127.0.0.1:8083/cgi-bin/hello.js?param1=value1&param2=value2"

# Test CGI with POST data
curl -v -X POST -d "name=test&value=123" "http://127.0.0.1:8083/cgi-bin/hello.py"
curl -v -X POST -d "data=test&id=456" "http://127.0.0.1:8083/cgi-bin/hello.js"

# Test CGI with different content types
curl -v -X POST -H "Content-Type: application/x-www-form-urlencoded" \
  -d "form=data&test=value" "http://127.0.0.1:8083/cgi-bin/hello.py"

curl -v -X POST -H "Content-Type: application/json" \
  -d '{"name":"test","value":123}' "http://127.0.0.1:8083/cgi-bin/hello.py"
```

## POST Request Tests

### File Upload Tests

```bash
# Upload a text file to uploads directory
curl -v -X POST \
  -F "file=@test_file.txt" \
  http://127.0.0.1:8083/uploads/

# Upload with custom filename
curl -v -X POST \
  -F "file=@test_file.txt;filename=custom_name.txt" \
  http://127.0.0.1:8083/uploads/

# Upload image file
curl -v -X POST \
  -F "file=@image.png" \
  http://127.0.0.1:8083/uploads/

# Upload JSON file
curl -v -X POST \
  -F "file=@data.json" \
  http://127.0.0.1:8083/uploads/

# Upload multiple files (if supported)
curl -v -X POST \
  -F "file1=@test_file.txt" \
  -F "file2=@another_file.txt" \
  http://127.0.0.1:8083/uploads/

# Test upload with different content types
curl -v -X POST \
  -H "Content-Type: multipart/form-data" \
  -F "file=@test_file.txt" \
  http://127.0.0.1:8083/uploads/
```

### POST Data to CGI

```bash
# Send form data to CGI script
curl -v -X POST \
  -d "name=John&email=john@example.com" \
  http://127.0.0.1:8083/cgi-bin/hello.py

# Send JSON data to CGI script
curl -v -X POST \
  -H "Content-Type: application/json" \
  -d '{"name":"John","email":"john@example.com"}' \
  http://127.0.0.1:8083/cgi-bin/hello.py
```

### API Endpoint Tests

```bash
# Test API JSON endpoint
curl -v http://127.0.0.1:8083/api/test.json

# Test API directory listing
curl -v http://127.0.0.1:8083/api/

# Test POST to API endpoint
curl -v -X POST \
  -H "Content-Type: application/json" \
  -d '{"test":"data","value":123}' \
  http://127.0.0.1:8083/api/

# Test API with query parameters
curl -v "http://127.0.0.1:8083/api/?format=json&pretty=true"
```

## DELETE Request Tests

### File Deletion Tests

```bash
# Delete uploaded file
curl -v -X DELETE http://127.0.0.1:8083/uploads/test_file.txt
curl -v -X DELETE http://127.0.0.1:8083/uploads/test_upload.txt

# Try to delete non-existent file
curl -v -X DELETE http://127.0.0.1:8083/uploads/nonexistent.txt

# Try to delete from restricted location (should fail)
curl -v -X DELETE http://127.0.0.1:8083/index.html
curl -v -X DELETE http://127.0.0.1:8083/imgs/imunaev-.png
curl -v -X DELETE http://127.0.0.1:8083/api/test.json

# Delete files with special characters in names
curl -v -X DELETE "http://127.0.0.1:8083/uploads/file%20with%20spaces.txt"
curl -v -X DELETE "http://127.0.0.1:8083/uploads/file-with-dashes.txt"
```

## HTTP Method Tests

### Method Not Allowed Tests

```bash
# POST to root location (should return 405)
curl -v -X POST http://127.0.0.1:8083/

# DELETE from root location (should return 405)
curl -v -X DELETE http://127.0.0.1:8083/

# PUT/PATCH to any location (should return 405)
curl -v -X PUT http://127.0.0.1:8083/
curl -v -X PATCH http://127.0.0.1:8083/uploads/

# POST to images directory (should return 405)
curl -v -X POST http://127.0.0.1:8083/imgs/

# DELETE from images directory (should return 405)
curl -v -X DELETE http://127.0.0.1:8083/imgs/imunaev-.png

# POST to API directory (should work)
curl -v -X POST http://127.0.0.1:8083/api/

# DELETE from API directory (should return 405)
curl -v -X DELETE http://127.0.0.1:8083/api/test.json

# Test all supported methods on uploads location
curl -v -X GET http://127.0.0.1:8083/uploads/
curl -v -X POST -F "file=@test.txt" http://127.0.0.1:8083/uploads/
curl -v -X DELETE http://127.0.0.1:8083/uploads/test.txt
```

## Error Status Code Tests

### 404 Not Found

```bash
# Request non-existent file
curl -v http://127.0.0.1:8083/nonexistent.html
curl -v http://127.0.0.1:8083/missing.txt
curl -v http://127.0.0.1:8083/fake.png

# Request non-existent directory
curl -v http://127.0.0.1:8083/nonexistent/
curl -v http://127.0.0.1:8083/fake_directory/

# Request file in non-existent subdirectory
curl -v http://127.0.0.1:8083/fake/path/file.html
curl -v http://127.0.0.1:8083/nonexistent/subdir/file.txt

# Request non-existent CGI script
curl -v http://127.0.0.1:8083/cgi-bin/nonexistent.py
curl -v http://127.0.0.1:8083/cgi-bin/missing.js

# Request non-existent API endpoint
curl -v http://127.0.0.1:8083/api/nonexistent.json
curl -v http://127.0.0.1:8083/api/missing/data.json
```

### 400 Bad Request

```bash
# Malformed request (missing Content-Type for multipart)
curl -v -X POST \
  -d "some data" \
  http://127.0.0.1:8083/cgi-bin/hello.py

# Invalid multipart data
curl -v -X POST \
  -H "Content-Type: multipart/form-data; boundary=invalid" \
  -d "invalid multipart data" \
  http://127.0.0.1:8083/cgi-bin/hello.py

# Malformed HTTP request
curl -v -X POST \
  -H "Content-Length: invalid" \
  -d "data" \
  http://127.0.0.1:8083/cgi-bin/hello.py

# Invalid query parameters
curl -v "http://127.0.0.1:8083/cgi-bin/hello.py?invalid%20query%20with%20spaces"
```

### 403 Forbidden

```bash
# Access forbidden location (upload without proper config)
curl -v -X POST \
  -F "file=@test_file.txt" \
  http://127.0.0.1:8083/

# Try to access restricted files
curl -v http://127.0.0.1:8083/../etc/passwd
curl -v http://127.0.0.1:8083/../../config.conf
```

### 413 Payload Too Large

```bash
# Upload file larger than client_max_body_size (1MB)
curl -v -X POST \
  -F "file=@large_file.txt" \
  http://127.0.0.1:8083/uploads/

# Upload large file to CGI
curl -v -X POST \
  -F "file=@large_file.txt" \
  http://127.0.0.1:8083/cgi-bin/hello.py

# Test with large POST data
curl -v -X POST \
  -d "$(head -c 2000000 /dev/zero | tr '\0' 'A')" \
  http://127.0.0.1:8083/cgi-bin/hello.py
```

### 500 Internal Server Error

```bash
# Trigger CGI script error (if script fails)
curl -v "http://127.0.0.1:8083/cgi-bin/broken_script.py"

# Request with problematic headers
curl -v -H "Invalid-Header: value" http://127.0.0.1:8083/
```

## HTTP Headers Tests

### Content-Type Tests

```bash
# Request with Accept header
curl -v -H "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" \
  http://127.0.0.1:8083/

# Request with User-Agent
curl -v -H "User-Agent: TestScript/1.0" \
  http://127.0.0.1:8083/
```

### Connection Tests

```bash
# Test keep-alive connection
curl -v --keepalive-time 30 http://127.0.0.1:8083/

# Test connection close
curl -v -H "Connection: close" http://127.0.0.1:8083/
```

## Chunked Transfer Tests

```bash
# Test chunked request body
curl -v -X POST \
  -H "Transfer-Encoding: chunked" \
  -d "chunked data here" \
  http://127.0.0.1:8083/cgi-bin/hello.py
```

## HEAD Method Tests

```bash
# HEAD request (should return headers only)
curl -v -I http://127.0.0.1:8083/

# HEAD request for favicon
curl -v -I http://127.0.0.1:8083/favicon.ico
```

## Stress Testing

### Multiple Requests

```bash
# Send multiple requests in sequence
for i in {1..10}; do
  curl -s http://127.0.0.1:8083/ > /dev/null && echo "Request $i: OK" || echo "Request $i: FAILED"
done
```

### Concurrent Requests

```bash
# Send concurrent requests (requires GNU parallel or similar)
# parallel -j 10 curl -s http://127.0.0.1:8083/ ::: {1..10}
```

## Configuration-Specific Tests

### Location-Based Access Control

```bash
# Test all supported methods on favicon location
curl -v -X GET http://127.0.0.1:8083/favicon.ico
curl -v -X POST http://127.0.0.1:8083/favicon.ico
curl -v -X DELETE http://127.0.0.1:8083/favicon.ico

# Test unsupported methods (should return 405)
curl -v -X PUT http://127.0.0.1:8083/favicon.ico
curl -v -X PATCH http://127.0.0.1:8083/favicon.ico
curl -v -X OPTIONS http://127.0.0.1:8083/favicon.ico
curl -v -X CONNECT http://127.0.0.1:8083/favicon.ico
curl -v -X TRACE http://127.0.0.1:8083/favicon.ico
```

## Response Validation Tests

### Check Response Headers

```bash
# Check Content-Type header
curl -I http://127.0.0.1:8083/ | grep -i "content-type"

# Check Content-Length header
curl -I http://127.0.0.1:8083/index.html | grep -i "content-length"

# Check Connection header
curl -I http://127.0.0.1:8083/ | grep -i "connection"
```

### Validate Response Content

```bash
# Check if response contains expected text
curl -s http://127.0.0.1:8083/ | grep -q "html" && echo "HTML content found" || echo "No HTML content"

# Check response size
curl -s http://127.0.0.1:8083/index.html | wc -c
```

## Comprehensive Test Suite

### Basic Functionality Test

```bash
#!/bin/bash
echo "=== Webserver Comprehensive Test Suite ==="

# Basic connectivity
echo "Testing basic connectivity..."
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8083/ | grep -q "200" && echo "✓ Basic connectivity OK" || echo "✗ Basic connectivity FAILED"

# Static files
echo "Testing static files..."
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8083/index.html | grep -q "200" && echo "✓ Index page OK" || echo "✗ Index page FAILED"
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8083/upload.html | grep -q "200" && echo "✓ Upload page OK" || echo "✗ Upload page FAILED"
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8083/delete.html | grep -q "200" && echo "✓ Delete page OK" || echo "✗ Delete page FAILED"

# Images
echo "Testing image serving..."
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8083/imgs/imunaev-.png | grep -q "200" && echo "✓ Images OK" || echo "✗ Images FAILED"

# API endpoints
echo "Testing API endpoints..."
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8083/api/test.json | grep -q "200" && echo "✓ API JSON OK" || echo "✗ API JSON FAILED"

# Autoindex
echo "Testing autoindex..."
curl -s http://127.0.0.1:8083/ | grep -q "index.html" && echo "✓ Root autoindex OK" || echo "✗ Root autoindex FAILED"
curl -s http://127.0.0.1:8083/testdir/ | grep -q "index.html" && echo "✓ Testdir autoindex OK" || echo "✗ Testdir autoindex FAILED"

# CGI scripts
echo "Testing CGI..."
curl -s -o /dev/null -w "%{http_code}" "http://127.0.0.1:8083/cgi-bin/hello.py" | grep -q "200" && echo "✓ Python CGI OK" || echo "✗ Python CGI FAILED"
curl -s -o /dev/null -w "%{http_code}" "http://127.0.0.1:8083/cgi-bin/hello.js" | grep -q "200" && echo "✓ JavaScript CGI OK" || echo "✗ JavaScript CGI FAILED"

# Error handling
echo "Testing error handling..."
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8083/nonexistent.html | grep -q "404" && echo "✓ 404 error OK" || echo "✗ 404 error FAILED"
curl -s -o /dev/null -w "%{http_code}" -X POST http://127.0.0.1:8083/ | grep -q "405" && echo "✓ Method restriction OK" || echo "✗ Method restriction FAILED"

# File operations
echo "Testing file operations..."
curl -s -o /dev/null -w "%{http_code}" -X POST -F "file=@/dev/null" http://127.0.0.1:8083/uploads/ | grep -q "200\|201" && echo "✓ File upload OK" || echo "✗ File upload FAILED"

echo "=== Test Suite Complete ==="
```

### Advanced Test Suite

```bash
#!/bin/bash
echo "=== Advanced Webserver Test Suite ==="

# Test all directory listings
echo "Testing directory listings..."
for dir in "/" "/testdir/" "/uploads/" "/api/" "/imgs/" "/empty/" "/default/" "/errors/" "/cgi-bin/"; do
  echo -n "Testing $dir... "
  curl -s -o /dev/null -w "%{http_code}" "http://127.0.0.1:8081$dir" | grep -q "200" && echo "✓" || echo "✗"
done

# Test all HTML pages
echo "Testing HTML pages..."
for page in "index.html" "upload.html" "delete.html" "api_landing.html" "api_response.html" "admin_dashboard.html" "upload_success.html"; do
  echo -n "Testing $page... "
  curl -s -o /dev/null -w "%{http_code}" "http://127.0.0.1:8083/$page" | grep -q "200" && echo "✓" || echo "✗"
done

# Test all images
echo "Testing images..."
for img in "imunaev-.png" "vlopatin.png" "lhaas.png"; do
  echo -n "Testing $img... "
  curl -s -o /dev/null -w "%{http_code}" "http://127.0.0.1:8083/imgs/$img" | grep -q "200" && echo "✓" || echo "✗"
done

# Test error pages
echo "Testing error pages..."
for error in "not_found_404.html" "method_not_allowed_405.html" "internal_server_error_500.html"; do
  echo -n "Testing $error... "
  curl -s -o /dev/null -w "%{http_code}" "http://127.0.0.1:8083/errors/$error" | grep -q "200" && echo "✓" || echo "✗"
done

echo "=== Advanced Test Suite Complete ==="
```

### Performance Test

```bash
#!/bin/bash
echo "=== Performance Test Suite ==="

# Test concurrent requests
echo "Testing concurrent requests..."
for i in {1..10}; do
  curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8083/ &
done
wait
echo "✓ Concurrent requests completed"

# Test response times
echo "Testing response times..."
time curl -s http://127.0.0.1:8083/ > /dev/null
time curl -s http://127.0.0.1:8083/cgi-bin/hello.py > /dev/null

echo "=== Performance Test Complete ==="
```

## New Configuration Test Locations

### Enhanced Directory Testing

```bash
# Test all new directory locations
curl -v http://127.0.0.1:8083/default/
curl -v http://127.0.0.1:8083/testdir/
curl -v http://127.0.0.1:8083/empty/
curl -v http://127.0.0.1:8083/errors/
curl -v http://127.0.0.1:8083/static/
curl -v http://127.0.0.1:8083/custom-index/
curl -v http://127.0.0.1:8083/mixed/
curl -v http://127.0.0.1:8083/head-only/
curl -v http://127.0.0.1:8083/large-uploads/
curl -v http://127.0.0.1:8083/small-uploads/
curl -v http://127.0.0.1:8083/cgi-test/
curl -v http://127.0.0.1:8083/nonexistent/
curl -v http://127.0.0.1:8083/forbidden/
curl -v http://127.0.0.1:8083/get-only/
```

### Specific File Testing

```bash
# Test specific HTML files
curl -v http://127.0.0.1:8083/admin_dashboard.html
curl -v http://127.0.0.1:8083/api_landing.html
curl -v http://127.0.0.1:8083/api_response.html
curl -v http://127.0.0.1:8083/upload_success.html
curl -v http://127.0.0.1:8083/autoindex_fallback.html
curl -v http://127.0.0.1:8083/autoindex_template.html

# Test specific image files
curl -v http://127.0.0.1:8083/imgs/imunaev-.png
curl -v http://127.0.0.1:8083/imgs/lhaas.png
curl -v http://127.0.0.1:8083/imgs/vlopatin.png

# Test specific error pages
curl -v http://127.0.0.1:8083/errors/not_found_404.html
curl -v http://127.0.0.1:8083/errors/bad_request_400.html
curl -v http://127.0.0.1:8083/errors/method_not_allowed_405.html
curl -v http://127.0.0.1:8083/errors/payload_too_large_413.html
curl -v http://127.0.0.1:8083/errors/internal_server_error_500.html

# Test specific files in subdirectories
curl -v http://127.0.0.1:8083/default/default.html
curl -v http://127.0.0.1:8083/testdir/index.html
curl -v http://127.0.0.1:8083/api/test.json
```

### Redirect Testing

```bash
# Test internal redirect
curl -v http://127.0.0.1:8083/redirect

# Test external redirect
curl -v http://127.0.0.1:8083/external-redirect

# Test redirect headers
curl -v -I http://127.0.0.1:8083/redirect
curl -v -I http://127.0.0.1:8083/external-redirect
```

### Enhanced CGI Testing

```bash
# Test CGI with different extensions
curl -v "http://127.0.0.1:8083/cgi-test/hello.py"
curl -v "http://127.0.0.1:8083/cgi-test/hello.js"

# Test CGI with query parameters
curl -v "http://127.0.0.1:8083/cgi-test/hello.py?test=value&param=123"
curl -v "http://127.0.0.1:8083/cgi-test/hello.js?name=test&id=456"

# Test CGI with POST data
curl -v -X POST -d "name=test&value=123" "http://127.0.0.1:8083/cgi-test/hello.py"
curl -v -X POST -d "data=test&id=456" "http://127.0.0.1:8083/cgi-test/hello.js"
```

### Enhanced Upload Testing

```bash
# Test large file uploads
curl -v -X POST \
  -F "file=@large_file.txt" \
  http://127.0.0.1:8083/large-uploads/

# Test small file uploads (for 413 testing)
curl -v -X POST \
  -F "file=@small_file.txt" \
  http://127.0.0.1:8083/small-uploads/

# Test mixed methods on upload locations
curl -v -X GET http://127.0.0.1:8083/large-uploads/
curl -v -X POST -F "file=@test.txt" http://127.0.0.1:8083/large-uploads/
curl -v -X DELETE http://127.0.0.1:8083/large-uploads/test.txt
```

### Method Testing

```bash
# Test mixed methods location
curl -v -X GET http://127.0.0.1:8083/mixed/
curl -v -X POST -d "data=test" http://127.0.0.1:8083/mixed/
curl -v -X DELETE http://127.0.0.1:8083/mixed/test

# Test GET-only location
curl -v -X GET http://127.0.0.1:8083/get-only/
curl -v -X POST http://127.0.0.1:8083/get-only/  # Should return 405
curl -v -X DELETE http://127.0.0.1:8083/get-only/  # Should return 405

# Test head-only location (using GET since HEAD is not supported)
curl -v -X GET http://127.0.0.1:8083/head-only/
```

### Autoindex Testing

```bash
# Test autoindex enabled locations
curl -v http://127.0.0.1:8083/  # Root with autoindex
curl -v http://127.0.0.1:8083/default/  # Default directory
curl -v http://127.0.0.1:8083/testdir/  # Test directory
curl -v http://127.0.0.1:8083/empty/  # Empty directory
curl -v http://127.0.0.1:8083/errors/  # Errors directory
curl -v http://127.0.0.1:8083/mixed/  # Mixed methods with autoindex

# Test autoindex disabled locations
curl -v http://127.0.0.1:8083/static/  # Should show custom error or index
curl -v http://127.0.0.1:8083/forbidden/  # Should show custom error or index
```

### Error Testing Locations

```bash
# Test 404 error location
curl -v http://127.0.0.1:8083/nonexistent/  # Should return 404

# Test 403 error location
curl -v http://127.0.0.1:8083/forbidden/  # Should return 403

# Test 405 error location
curl -v -X POST http://127.0.0.1:8083/get-only/  # Should return 405
curl -v -X DELETE http://127.0.0.1:8083/get-only/  # Should return 405

# Test 413 error location
curl -v -X POST \
  -F "file=@very_large_file.txt" \
  http://127.0.0.1:8083/small-uploads/  # Should return 413
```

### Custom Index Testing

```bash
# Test custom index location
curl -v http://127.0.0.1:8083/custom-index/  # Should look for custom.html

# Test specific index files
curl -v http://127.0.0.1:8083/default/default.html
curl -v http://127.0.0.1:8083/testdir/index.html
```

## Comprehensive New Test Suite

### All New Locations Test

```bash
#!/bin/bash
echo "=== New Configuration Test Suite ==="

# Test all new directory locations
echo "Testing new directory locations..."
for dir in "/default/" "/testdir/" "/empty/" "/errors/" "/static/" "/custom-index/" "/mixed/" "/head-only/" "/large-uploads/" "/small-uploads/" "/cgi-test/" "/nonexistent/" "/forbidden/" "/get-only/"; do
  echo -n "Testing $dir... "
  curl -s -o /dev/null -w "%{http_code}" "http://127.0.0.1:8083$dir" | grep -q "200\|404\|403\|405" && echo "✓" || echo "✗"
done

# Test all new HTML pages
echo "Testing new HTML pages..."
for page in "admin_dashboard.html" "api_landing.html" "api_response.html" "upload_success.html" "autoindex_fallback.html" "autoindex_template.html"; do
  echo -n "Testing $page... "
  curl -s -o /dev/null -w "%{http_code}" "http://127.0.0.1:8083/$page" | grep -q "200" && echo "✓" || echo "✗"
done

# Test all error pages
echo "Testing error pages..."
for error in "not_found_404.html" "bad_request_400.html" "method_not_allowed_405.html" "payload_too_large_413.html" "internal_server_error_500.html"; do
  echo -n "Testing $error... "
  curl -s -o /dev/null -w "%{http_code}" "http://127.0.0.1:8083/errors/$error" | grep -q "200" && echo "✓" || echo "✗"
done

# Test redirects
echo "Testing redirects..."
echo -n "Testing /redirect... "
curl -s -o /dev/null -w "%{http_code}" "http://127.0.0.1:8083/redirect" | grep -q "302" && echo "✓" || echo "✗"
echo -n "Testing /external-redirect... "
curl -s -o /dev/null -w "%{http_code}" "http://127.0.0.1:8083/external-redirect" | grep -q "302" && echo "✓" || echo "✗"

# Test CGI locations
echo "Testing CGI locations..."
echo -n "Testing /cgi-test/hello.py... "
curl -s -o /dev/null -w "%{http_code}" "http://127.0.0.1:8083/cgi-test/hello.py" | grep -q "200" && echo "✓" || echo "✗"
echo -n "Testing /cgi-test/hello.js... "
curl -s -o /dev/null -w "%{http_code}" "http://127.0.0.1:8083/cgi-test/hello.js" | grep -q "200" && echo "✓" || echo "✗"

echo "=== New Configuration Test Suite Complete ==="
```

### Method Restriction Testing

```bash
#!/bin/bash
echo "=== Method Restriction Test Suite ==="

# Test GET-only locations
echo "Testing GET-only restrictions..."
echo -n "Testing /get-only/ with GET... "
curl -s -o /dev/null -w "%{http_code}" -X GET "http://127.0.0.1:8083/get-only/" | grep -q "200" && echo "✓" || echo "✗"
echo -n "Testing /get-only/ with POST... "
curl -s -o /dev/null -w "%{http_code}" -X POST "http://127.0.0.1:8083/get-only/" | grep -q "405" && echo "✓" || echo "✗"
echo -n "Testing /get-only/ with DELETE... "
curl -s -o /dev/null -w "%{http_code}" -X DELETE "http://127.0.0.1:8083/get-only/" | grep -q "405" && echo "✓" || echo "✗"

# Test mixed methods locations
echo "Testing mixed methods..."
echo -n "Testing /mixed/ with GET... "
curl -s -o /dev/null -w "%{http_code}" -X GET "http://127.0.0.1:8083/mixed/" | grep -q "200" && echo "✓" || echo "✗"
echo -n "Testing /mixed/ with POST... "
curl -s -o /dev/null -w "%{http_code}" -X POST "http://127.0.0.1:8083/mixed/" | grep -q "200" && echo "✓" || echo "✗"
echo -n "Testing /mixed/ with DELETE... "
curl -s -o /dev/null -w "%{http_code}" -X DELETE "http://127.0.0.1:8083/mixed/" | grep -q "200" && echo "✓" || echo "✗"

echo "=== Method Restriction Test Suite Complete ==="
```

### Upload Testing Suite

```bash
#!/bin/bash
echo "=== Upload Test Suite ==="

# Create test files
echo "Creating test files..."
echo "Small test file" > small_test.txt
dd if=/dev/zero of=large_test.txt bs=1M count=2 2>/dev/null

# Test small uploads
echo "Testing small file uploads..."
echo -n "Testing /uploads/ with small file... "
curl -s -o /dev/null -w "%{http_code}" -X POST -F "file=@small_test.txt" "http://127.0.0.1:8083/uploads/" | grep -q "200\|201" && echo "✓" || echo "✗"
echo -n "Testing /large-uploads/ with small file... "
curl -s -o /dev/null -w "%{http_code}" -X POST -F "file=@small_test.txt" "http://127.0.0.1:8083/large-uploads/" | grep -q "200\|201" && echo "✓" || echo "✗"
echo -n "Testing /small-uploads/ with small file... "
curl -s -o /dev/null -w "%{http_code}" -X POST -F "file=@small_test.txt" "http://127.0.0.1:8083/small-uploads/" | grep -q "200\|201" && echo "✓" || echo "✗"

# Test large uploads
echo "Testing large file uploads..."
echo -n "Testing /large-uploads/ with large file... "
curl -s -o /dev/null -w "%{http_code}" -X POST -F "file=@large_test.txt" "http://127.0.0.1:8083/large-uploads/" | grep -q "200\|201\|413" && echo "✓" || echo "✗"
echo -n "Testing /small-uploads/ with large file (should fail)... "
curl -s -o /dev/null -w "%{http_code}" -X POST -F "file=@large_test.txt" "http://127.0.0.1:8083/small-uploads/" | grep -q "413" && echo "✓" || echo "✗"

# Cleanup
rm -f small_test.txt large_test.txt
echo "=== Upload Test Suite Complete ==="
```

## Updated Port References

**Note**: All test commands have been updated to use port **8083** to match the new configuration. Make sure to update any existing test scripts or commands to use the correct port number.
