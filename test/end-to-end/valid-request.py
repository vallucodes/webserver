import socket

HOST = '127.0.0.1'
PORT = 8080

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))

request = (
	"POST /uploads HTTP/1.1\r\n"
	f"Host: {HOST}:{PORT}\r\n"
	"Content-Type: text/plain\r\n"
	"Content-Length: 5\r\n"  # length of "12345"
	"\r\n"
	"12345"
)

with socket.create_connection((HOST, PORT)) as sock:
	sock.sendall(request.encode())
	# Receive the response
	response = sock.recv(8192)
	print("=== Server Response ===")
	print(response.decode())
