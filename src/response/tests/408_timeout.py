import socket
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('127.0.0.1', 8080))

# Send partial request (missing \r\n\r\n)
partial_request = "GET / HTTP/1.1\r\nHost: localhost:8080\r\n"
sock.send(partial_request.encode())

# Wait 6+ seconds for timeout
time.sleep(6)
response = sock.recv(4096).decode()
print(response)  # Should show "408 Request Timeout"
