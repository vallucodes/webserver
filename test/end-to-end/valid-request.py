import socket

HOST = '127.0.0.1'
PORT = 7070

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))

request = (
	"GET /uploads HTTP/1.1\r\n"
	"Content-Length: 5\r\n"
	"\r\n"
	"12345"
)

sock.sendall(request.encode())

# Receive the response
response = b""
while True:
	data = sock.recv(1024)
	if not data:
		break
	response += data

print(response.decode())

sock.close()
