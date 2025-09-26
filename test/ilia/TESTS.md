# Basic Webserver Tests - curl Commands

## 1. GET Requests → Should Work
```bash
curl -v http://127.0.0.1:8080/
curl -v http://127.0.0.1:8080/index.html
curl -v http://127.0.0.1:8080/delete.html
curl -v http://127.0.0.1:8080/favicon.ico
curl -v http://127.0.0.1:8080/uploads/
curl -v http://127.0.0.1:8080/imgs/
curl -v http://127.0.0.1:8080/cgi-bin/
curl -v "http://127.0.0.1:8080/cgi-bin/hello.py"
curl -v "http://127.0.0.1:8080/cgi-bin/hello.js"
curl -v -L http://127.0.0.1:8080/old # to see the second real request
```

## 2. POST Requests → Should Work
```bash
echo "test content" > test.txt
curl -v -X POST -F "file=@test.txt" http://127.0.0.1:8080/uploads/
curl -v -X POST -d "name=test&value=123" "http://127.0.0.1:8080/cgi-bin/hello.py"
curl -v -X POST -H "Content-Type: application/json" -d '{"name":"test","value":123}' "http://127.0.0.1:8080/cgi-bin/hello.py"
rm -f test.txt
```

## 3. DELETE Requests → Should Work
```bash
echo "delete me" > delete_test.txt
curl -v -X POST -F "file=@delete_test.txt" http://127.0.0.1:8080/uploads/
curl -v -X DELETE http://127.0.0.1:8080/uploads/delete_test.txt
curl -v -X DELETE http://127.0.0.1:8080/uploads/nonexistent.txt
rm -f delete_test.txt
```

## 4. UNKNOWN Requests → Should Not Crash
```bash
curl -v -X PUT http://127.0.0.1:8080/
curl -v -X PATCH http://127.0.0.1:8080/uploads/
curl -v -X OPTIONS http://127.0.0.1:8080/
```

## 5. Status Code Checks (8080)
```bash
# 2xx Success Codes
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/ # 200 OK
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/index.html # 200 OK
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/favicon.ico # 200 OK
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/cgi-bin/hello.py # 200 OK
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/cgi-bin/hello.js # 200 OK
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/uploads/ # 200 OK (directory listing)
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/imgs/ # 200 OK (directory listing)

# 3xx Redirection Codes
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/old # 302
  
# 4xx Client Error Codes
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/nonexistent.html # 404 Not Found
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/cgi-bin/nonexistent.py # 404 Not Found
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/uploads/nonexistent.txt # 404 Not Found
curl -s -o /dev/null -w "%{http_code}" -X PUT http://127.0.0.1:8080/ # 405 Method Not Allowed
curl -s -o /dev/null -w "%{http_code}" -X PATCH http://127.0.0.1:8080/uploads/ # 405 Method Not Allowed
curl -s -o /dev/null -w "%{http_code}" -X OPTIONS http://127.0.0.1:8080/ # 405 Method Not Allowed
curl -s -o /dev/null -w "%{http_code}" -X DELETE http://127.0.0.1:8080/uploads/nonexistent.txt # 404 Not Found

# 5xx Server Error Codes
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/cgi-bin/inf.py # 504 Gateway Timeout

# Multi-server status code checks
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8081/ # 200 OK
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8082/ # 200 OK
```

## 6. Upload File and Get It Back
```bash
echo "upload test $(date)" > upload_test.txt
curl -v -X POST -F "file=@upload_test.txt" http://127.0.0.1:8080/uploads/
curl -v http://127.0.0.1:8080/uploads/upload_test.txt
rm -f upload_test.txt
```
# 7. Infinitive loop
```bash
curl -v http://127.0.0.1:8080/cgi-bin/inf.py
```

## Multi-Server Tests (8080, 8081, 8082)
```bash
curl -v http://127.0.0.1:8080/
curl -v http://127.0.0.1:8081/
curl -v http://127.0.0.1:8082/
```
```
