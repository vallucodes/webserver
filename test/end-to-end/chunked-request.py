import socket
import time

# Server configuration
HOST = '127.0.0.1'
PORT = 8081

chunks = [
	"\r\n123456789RLFAA",
]

request_headers = (
	"GET /uploads HTTP/1.1\r\n"
	f"Host: {HOST}:{PORT}\r\n"
	"Transfer-Encoding: chunked\r\n"
	"Connection: keep-alive\r\n"
	"Content-Type: text/plain\r\n"
	"\r\n"
)

with socket.create_connection((HOST, PORT)) as sock:
	# Send headers first
	sock.sendall(request_headers.encode())

	# Send chunks (might be invalid because sent in parts)
	for i, chunk in enumerate(chunks):
		chunk_bytes = chunk.encode('utf-8')
		size = f"{len(chunk_bytes):X}"
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
	time.sleep(2)
