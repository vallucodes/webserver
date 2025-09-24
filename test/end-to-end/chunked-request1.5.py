import socket
import time

# Server configuration
HOST = '127.0.0.1'
PORT = 8081

# Define tricky chunks
chunks = [
	"H",                       # 1-byte chunk
	"ello, world!\n",          # normal chunk with newline inside
]

# Build the raw HTTP request headers
request_headers = (
	"POST /uploads HTTP/1.1\r\n"
	"Host: localhost\r\n"
	"Transfer-Encoding: chunked\r\n"
	"\r\n"
)

with socket.create_connection((HOST, PORT)) as sock:
	# Send headers first
	sock.sendall(request_headers.encode())

	for chunk in chunks:
		chunk_bytes = chunk.encode('utf-8')
		size = f"{len(chunk_bytes):X}"     # hex chunk size
		frame = f"{size}\r\n".encode() + chunk_bytes + b"\r\n"
		sock.sendall(frame)
		time.sleep(1)

	# Final zero-length chunk to indicate end of body
	sock.sendall(b"0\r\n\r\n")

	# Send some extra junk (half of another request)
	sock.sendall(b"GET /incomplete HTTP/1.2\r\nHost: test\r\n")

	# Read and print the response from the server
	response = sock.recv(8192)
	print("=== Server Response ===")
	print(response.decode(errors='replace'))
