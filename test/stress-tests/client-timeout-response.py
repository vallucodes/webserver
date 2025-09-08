# Test when client send request but never opens POLLIN to receive request, but not disconnected

import socket
import time

HOST = '127.0.0.1'
PORT = 2052

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))

request = (
	"GET / HTTP/1.1\r\n"
	"Host: 127.0.0.1\r\n"
	"Connection: keep-alive\r\n"
	"\r\n"
)
sock.sendall(request.encode())

print("Request sent, now sleeping without reading response...")
time.sleep(600)  # Sleep long enough to trigger server timeout
