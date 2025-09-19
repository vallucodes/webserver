import socket
import time

# Server configuration
HOST = '127.0.0.1'
PORT = 7070

# Define tricky chunks
chunks = [
	"\r\n123456789RLFAA",             # chunk containing CRLF inside body
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
	# Send headers first
	sock.sendall(request_headers.encode())

	for i, chunk in enumerate(chunks):
		chunk_bytes = chunk.encode('utf-8')         # encode first
		size = f"{len(chunk_bytes):X}"              # chunk size in bytes
		sock.sendall(f"{size}\r\n".encode())
		sock.sendall(chunk_bytes)
		sock.sendall(b"\r\n")

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
