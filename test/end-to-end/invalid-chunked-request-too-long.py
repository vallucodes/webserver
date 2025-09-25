# set max body size in confign to 100, then test this with "1" and without

import socket
import time

# Server configuration
HOST = '127.0.0.1'
PORT = 8082

# Define tricky chunks
chunks = [
	"1234567890",
	"1234567890",
	"1234567890",
	"1234567890",
	"1234567890",
	"1234567890",
	"1234567890",
	"1234567890",
	"1234567890",
	"1234567890",
	"1",
]

# Build the raw HTTP request headers
request_headers = (
	"POST /uploads HTTP/1.1\r\n"
	f"Host: {HOST}:{PORT}\r\n"
	"Transfer-Encoding: chunked\r\n"
	"Content-Type: text/plain\r\n"
	"\r\n"
)

with socket.create_connection((HOST, PORT)) as sock:
	sock.sendall(request_headers.encode())

	for i, chunk in enumerate(chunks):
		chunk_bytes = chunk.encode('utf-8')
		size = f"{len(chunk_bytes):X}"
		frame = f"{size}\r\n".encode() + chunk_bytes + b"\r\n"
		sock.sendall(frame)

		# Optional: simulate network delays
		if i % 2 == 0:
			time.sleep(0.2)

	# Final zero-length chunk to indicate end of body
	sock.sendall(b"0\r\n\r\n")
	print("=== Final zero-length chunk sent ===")

	# Read and print the response from the server
	response = sock.recv(8192)
	print("=== Server Response ===")
	print(response.decode(errors='replace'))
