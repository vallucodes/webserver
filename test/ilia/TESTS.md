## 1. BASIC GET TESTS

### Server 1 (Port 8080) - Full Functionality
```bash
# Root directory
curl -v http://127.0.0.1:8080/

# Index file
curl -v http://127.0.0.1:8080/index.html

# Static files
curl -v http://127.0.0.1:8080/favicon.ico
curl -v http://127.0.0.1:8080/upload.html
curl -v http://127.0.0.1:8080/delete.html

# Directory listings
curl -v http://127.0.0.1:8080/uploads/
curl -v http://127.0.0.1:8080/imgs/

# CGI scripts
curl -v http://127.0.0.1:8080/cgi-bin/hello.py
curl -v http://127.0.0.1:8080/cgi-bin/hello.js

# Redirection (should return 302)
curl -v http://127.0.0.1:8080/old
curl -v -L http://127.0.0.1:8080/old  # Follow redirect

# Not found 404
curl -v http://127.0.0.1:8080/cgi-bin/
curl -v http://127.0.0.1:8080/noexist.html
curl -v http://127.0.0.1:8080/nodir/

```
### Multi-Server GET Tests
```bash
# Test all servers
curl -v http://127.0.0.1:8080/  # Server 1 - Full
curl -v http://127.0.0.1:8081/  # Server 2 - GET/DELETE only
curl -v http://127.0.0.1:8082/  # Server 3 - GET/POST only
curl -v http://127.0.0.1:8083/  # Server 4 - Simple site2
curl -v http://127.0.0.1:8084/  # Server 5 - Simple site3
```

---

## 2. POST/UPLOAD TESTS

### Server 1 (Port 8080) - POST Allowed
```bash
# File upload
echo "test content $(date)" > test.txt
curl -v -X POST -F "file=@test.txt" http://127.0.0.1:8080/uploads/
curl -v http://127.0.0.1:8080/uploads/test.txt  # Verify upload

# CGI POST with form data
curl -v -X POST -d "name=test&value=123" http://127.0.0.1:8080/cgi-bin/hello.py

# CGI POST with JSON
curl -v -X POST -H "Content-Type: application/json" -d '{"name":"test","value":123}' http://127.0.0.1:8080/cgi-bin/hello.py

# Large file upload (testing client_max_body_size)

```bash
# This should return 404 or 405 (POST not allowed)
echo "test content" > test2.txt
curl -v -X POST -F "file=@test2.txt" http://127.0.0.1:8081/uploads/
rm -f test2.txt
```

### Server 3 (Port 8082) - POST Allowed
```bash
# File upload should work
echo "test content" > test3.txt
curl -v -X POST -F "file=@test3.txt" http://127.0.0.1:8082/uploads/
curl -v http://127.0.0.1:8082/uploads/test3.txt  # Verify upload
rm -f test3.txt
```

---

## 3. DELETE TESTS

### Server 1 (Port 8080) - DELETE Allowed
```bash
# Upload a file first
echo "delete me" > delete_test.txt
curl -v -X POST -F "file=@delete_test.txt" http://127.0.0.1:8080/uploads/

# Delete the file
curl -v -X DELETE http://127.0.0.1:8080/uploads/delete_test.txt

# Try to delete nonexistent file (should return 404)
curl -v -X DELETE http://127.0.0.1:8080/uploads/nonexistent.txt
```

### Server 2 (Port 8081) - DELETE Allowed
```bash
# DELETE should work (but file doesn't exist, should return 404)
curl -v -X DELETE http://127.0.0.1:8081/uploads/nonexistent.txt
```

### Server 3 (Port 8082) - DELETE Should Be Denied
```bash
# This should return 404 or 405 (DELETE not allowed)
curl -v -X DELETE http://127.0.0.1:8082/uploads/nonexistent.txt
```

---

## 4. ERROR HANDLING TESTS

### 404 Not Found
```bash
curl -v http://127.0.0.1:8080/nonexistent.html
curl -v http://127.0.0.1:8080/cgi-bin/nonexistent.py
curl -v http://127.0.0.1:8080/uploads/nonexistent.txt
```

### 405 Method Not Allowed
```bash
# These should return 405
curl -v -X PUT http://127.0.0.1:8080/
curl -v -X PATCH http://127.0.0.1:8080/uploads/
curl -v -X OPTIONS http://127.0.0.1:8080/
curl -v -X HEAD http://127.0.0.1:8080/
```

### 403 Forbidden / 404 Not Found / 405 Method Not Allowed
```bash
# Test 404 Not Found - POST to non-existent location (should be 404)
curl -v -X POST -F "file=@test.txt" http://127.0.0.1:8080/noupload/
curl -v -X POST -F "file=@test.txt" http://127.0.0.1:8080/nonexistent/

# Test 404 Not Found - DELETE from non-existent location (should be 404)
curl -v -X DELETE http://127.0.0.1:8080/noupload/test.txt
curl -v -X DELETE http://127.0.0.1:8080/nonexistent/test.tx
```

### 413 Payload Too Large
```bash
# Create a large file (>client_max_body_size) to test size limit
# Creates 20KB file (exceeds 10KB limit)
dd if=/dev/zero of=large_file.txt bs=1K count=2000
curl -v -X POST -F "file=@large_file.txt" http://127.0.0.1:8080/uploads/
rm -f large_file.txt

# Test with file within client_max_body_size (should work)
dd if=/dev/zero of=small_file.txt bs=1K count=5  # Creates 5KB file
curl -v -X POST -F "file=@small_file.txt" http://127.0.0.1:8080/uploads/
curl -v -X DELETE http://127.0.0.1:8080/uploads/small_file.txt
rm -f small_file.txt

```

### 500 Server Error (CGI timeout)
```bash
# This should return 504 Gateway Timeout after timeout
curl -v http://127.0.0.1:8080/cgi-bin/inf.py
```

---

## 5. STATUS CODE VERIFICATION

### Quick Status Checks
```bash
# Success codes (200)
curl -s -o /dev/null -w "Root: %{http_code}\n" http://127.0.0.1:8080/
curl -s -o /dev/null -w "Index: %{http_code}\n" http://127.0.0.1:8080/index.html
curl -s -o /dev/null -w "Favicon: %{http_code}\n" http://127.0.0.1:8080/favicon.ico
curl -s -o /dev/null -w "Uploads: %{http_code}\n" http://127.0.0.1:8080/uploads/
curl -s -o /dev/null -w "CGI: %{http_code}\n" http://127.0.0.1:8080/cgi-bin/hello.py

# Redirection (302)
curl -s -o /dev/null -w "Redirect: %{http_code}\n" http://127.0.0.1:8080/old

# Error codes (404)
curl -s -o /dev/null -w "Not Found: %{http_code}\n" http://127.0.0.1:8080/nonexistent.html

# Method not allowed (405/400)
curl -s -o /dev/null -w "PUT: %{http_code}\n" -X PUT http://127.0.0.1:8080/
curl -s -o /dev/null -w "PATCH: %{http_code}\n" -X PATCH http://127.0.0.1:8080/uploads/

# Not Found (404) - non-existent paths
curl -s -o /dev/null -w "POST to noupload: %{http_code}\n" -X POST -F "file=@test.txt" http://127.0.0.1:8080/noupload/

# Method Not Allowed (405) - existing path but wrong method
curl -s -o /dev/null -w "POST to cgi-bin: %{http_code}\n" -X POST -F "file=@test.txt" http://127.0.0.1:8080/cgi-bin/

# Payload too large (413) / Bad Request (400) - create large file first
dd if=/dev/zero of=large_test.txt bs=1K count=20 2>/dev/null
curl -s -o /dev/null -w "Large file upload: %{http_code}\n" -X POST -F "file=@large_test.txt" http://127.0.0.1:8080/uploads/
rm -f large_test.txt

# Multi-server status check
curl -s -o /dev/null -w "Server 1: %{http_code}\n" http://127.0.0.1:8080/
curl -s -o /dev/null -w "Server 2: %{http_code}\n" http://127.0.0.1:8081/
curl -s -o /dev/null -w "Server 3: %{http_code}\n" http://127.0.0.1:8082/
curl -s -o /dev/null -w "Server 4: %{http_code}\n" http://127.0.0.1:8083/
curl -s -o /dev/null -w "Server 5: %{http_code}\n" http://127.0.0.1:8084/
```

---

## 6. COMPLETE WORKFLOW TESTS

### Upload → Verify → Delete Workflow
```bash
# 1. Upload a file
echo "workflow test $(date)" > workflow_test.txt
curl -v -X POST -F "file=@workflow_test.txt" http://127.0.0.1:8080/uploads/

# 2. Verify the file exists
curl -v http://127.0.0.1:8080/uploads/workflow_test.txt

# 3. Check directory listing includes the file
curl -v http://127.0.0.1:8080/uploads/

# 4. Delete the file
curl -v -X DELETE http://127.0.0.1:8080/uploads/workflow_test.txt

# 5. Verify file is gone
curl -v http://127.0.0.1:8080/uploads/workflow_test.txt  # Should return 404

# Cleanup
rm -f workflow_test.txt
```

### CGI Environment Test
```bash
# Test CGI with different methods and data
curl -v http://127.0.0.1:8080/cgi-bin/hello.py
curl -v -X POST -d "test=data" http://127.0.0.1:8080/cgi-bin/hello.py
curl -v -X POST -H "Content-Type: application/json" -d '{"test":"json"}' http://127.0.0.1:8080/cgi-bin/hello.py
```
---
