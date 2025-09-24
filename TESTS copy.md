# Basic Webserver Tests - mands

## 1. GET Requests → Should Work
```bash
http://127.0.0.1:8080/
http://127.0.0.1:8080/index.html
http://127.0.0.1:8080/index.html
http://127.0.0.1:8080/delete.html
http://127.0.0.1:8080/favicon.ico
http://127.0.0.1:8080/uploads/
http://127.0.0.1:8080/imgs/
http://127.0.0.1:8080/cgi-bin/ 
http://127.0.0.1:8080/cgi-bin/hello.py
http://127.0.0.1:8080/cgi-bin/hello.js
http://127.0.0.1:8080/old # to see the second real request
```

## 2. POST Requests → Should Work
```bash
echo "test content" > test.txt
POST -F "file=@test.txt" http://127.0.0.1:8080/uploads/
POST -d "name=test&value=123" "http://127.0.0.1:8080/cgi-bin/hello.py"
POST -H "Content-Type: application/json" -d '{"name":"test","value":123}' "http://127.0.0.1:8080/cgi-bin/hello.py"
rm -f test.txt
```

## 3. DELETE Requests → Should Work
```bash
echo "delete me" > delete_test.txt
-X POST -F "file=@delete_test.txt" http://127.0.0.1:8080/uploads/
-X DELETE http://127.0.0.1:8080/uploads/delete_test.txt
-X DELETE http://127.0.0.1:8080/uploads/nonexistent.txt
rm -f delete_test.txt
```

## 4. UNKNOWN Requests → Should Not Crash
```bash
-X PUT http://127.0.0.1:8080/
-X PATCH http://127.0.0.1:8080/uploads/
-X OPTIONS http://127.0.0.1:8080/
```

## 5. Status Code Checks
```bash
-o /dev/null -w "%{http_code}" http://127.0.0.1:8080/ # 200
-o /dev/null -w "%{http_code}" http://127.0.0.1:8080/nonexistent.html #404
-o /dev/null -w "%{http_code}" -X PUT http://127.0.0.1:8080/ #400
```

## 6. Upload File and Get It Back
```bash
echo "upload test $(date)" > upload_test.txt
-X POST -F "file=@upload_test.txt" http://127.0.0.1:8080/uploads/
http://127.0.0.1:8080/uploads/upload_test.txt
rm -f upload_test.txt
```

## Multi-Server Tests (8080, 8081, 8082)
```bash
http://127.0.0.1:8080/
http://127.0.0.1:8081/
http://127.0.0.1:8082/
```
```
