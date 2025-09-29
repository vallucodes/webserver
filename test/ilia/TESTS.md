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
# These should return 405 or 400 (unsupported methods)
curl -v -X PUT http://127.0.0.1:8080/
curl -v -X PATCH http://127.0.0.1:8080/uploads/
curl -v -X OPTIONS http://127.0.0.1:8080/
curl -v -X HEAD http://127.0.0.1:8080/
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

## 7. STRESS/CONCURRENT TESTS

### Multiple Concurrent Requests
```bash
# Run multiple requests simultaneously
for i in {1..5}; do
    curl -s -o /dev/null -w "Request $i: %{http_code}\n" http://127.0.0.1:8080/ &
done
wait
```

### Mixed Method Testing
```bash
# Test different methods on same server
curl -s -o /dev/null -w "GET: %{http_code}\n" http://127.0.0.1:8080/uploads/
curl -s -o /dev/null -w "POST: %{http_code}\n" -X POST -F "file=@test.txt" http://127.0.0.1:8080/uploads/
curl -s -o /dev/null -w "DELETE: %{http_code}\n" -X DELETE http://127.0.0.1:8080/uploads/test.txt
```

---

## 8. EXPECTED RESULTS SUMMARY

| Test | Server 1 (8080) | Server 2 (8081) | Server 3 (8082) | Server 4 (8083) | Server 5 (8084) |
|------|----------------|----------------|----------------|----------------|----------------|
| GET / | PASS 200 | PASS 200 | PASS 200 | PASS 200 | PASS 200 |
| POST /uploads/ | PASS 201 | FAIL 404/405 | PASS 201 | N/A | N/A |
| DELETE /uploads/ | PASS 204 | PASS 204 | FAIL 404/405 | N/A | N/A |
| CGI scripts | PASS 200 | N/A | N/A | N/A | N/A |
| Redirect /old | PASS 302 | N/A | N/A | N/A | N/A |

---

## TROUBLESHOOTING

### Common Issues
1. **Server not responding**: Check if webserver is running with `ps aux | grep webserv`
2. **Permission denied**: Ensure test files are readable and upload directory exists
3. **CGI scripts not working**: Check if Python/Node.js are installed and scripts are executable
4. **Unexpected status codes**: Verify configuration file syntax and server logs

### Debug Commands
```bash
# Check server processes
ps aux | grep webserv

# Check server logs (if available)
tail -f webserver.log

# Test connectivity
telnet 127.0.0.1 8080
telnet 127.0.0.1 8081
telnet 127.0.0.1 8082
telnet 127.0.0.1 8083
telnet 127.0.0.1 8084

# Check file permissions
ls -la www/
ls -la www/uploads/
ls -la www/cgi-bin/
```

---

## CLEANUP
```bash
# Remove test files
rm -f test.txt test2.txt test3.txt delete_test.txt workflow_test.txt large_test.txt upload_test.txt

# Stop webserver (if running in foreground)
# Press Ctrl+C or kill the process
```
