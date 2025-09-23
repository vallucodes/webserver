# Basic Webserver Tests - curl Commands

## 1. GET Requests → Should Work
```bash
curl -v http://127.0.0.1:8080/
curl -v http://127.0.0.1:8080/index.html
curl -v http://127.0.0.1:8080/index.html
curl -v http://127.0.0.1:8080/delete.html
curl -v http://127.0.0.1:8080/favicon.ico
curl -v http://127.0.0.1:8080/uploads/
curl -v http://127.0.0.1:8080/imgs/
curl -v http://127.0.0.1:8080/cgi-bin/ #bug if http://127.0.0.1:8080//////cgi-bin/
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

## 5. Status Code Checks
```bash
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/ # 200
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/nonexistent.html #404
curl -s -o /dev/null -w "%{http_code}" -X PUT http://127.0.0.1:8080/ #400
```

## 6. Upload File and Get It Back
```bash
echo "upload test $(date)" > upload_test.txt
curl -v -X POST -F "file=@upload_test.txt" http://127.0.0.1:8080/uploads/
curl -v http://127.0.0.1:8080/uploads/upload_test.txt
rm -f upload_test.txt
```

## Multi-Server Tests (8080, 8081, 8082)
```bash
curl -v http://127.0.0.1:8080/
curl -v http://127.0.0.1:8081/
curl -v http://127.0.0.1:8082/
```
```
