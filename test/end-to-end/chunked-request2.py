import socket
import time

# Server configuration
HOST = '127.0.0.1'
PORT = 8082

# Define tricky chunks
chunks = [
	"H",
	"ello, world!\n",
	"fgfgfg",
	"1234567890",
	"Between-CLRF",
]

request_headers = (
	"GET /uploads HTTP/1.1\r\n"
	"Host: localhost\r\n"
	"Transfer-Encoding: chunked\r\n"
	"Content-Type: text/plain\r\n"
	"\r\n"
)

with socket.create_connection((HOST, PORT)) as sock:
	# Send headers first
	sock.sendall(request_headers.encode())

	# Send chunks (might be invalid because sent in parts)
	for chunk in chunks:
		chunk_bytes = chunk.encode('utf-8')
		size = f"{len(chunk_bytes):X}"
		sock.sendall(f"{size}\r\n".encode())
		sock.sendall(chunk_bytes)
		sock.sendall(b"\r\n")
		# time.sleep(1)

	# Final zero-length chunk to indicate end of body
	sock.sendall(b"0\r\n\r\n")

	# Read and print the response from the server
	response = sock.recv(8192)
	print("=== Server Response ===")
	print(response.decode(errors='replace'))
