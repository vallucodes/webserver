# set max body size in confign to 100, then test this with "1" and without.
# remember to change Content-Length field

import socket

HOST = '127.0.0.1'
PORT = 8081

request = (
	"POST /uploads HTTP/1.1\r\n"
	f"Host: {HOST}:{PORT}\r\n"
	"Content-Type: text/plain\r\n"
	"Content-Length: 101\r\n"
	"\r\n"
	"1234567890"
	"1234567890"
	"1234567890"
	"1234567890"
	"1234567890"
	"1234567890"
	"1234567890"
	"1234567890"
	"1234567890"
	"1234567890"
	"1"
)

with socket.create_connection((HOST, PORT)) as sock:
	sock.sendall(request.encode())
	# Receive the response
	response = sock.recv(8192)
	print("=== Server Response ===")
	print(response.decode())
