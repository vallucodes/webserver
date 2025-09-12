import socket

HOST = '127.0.0.1'
PORT = 7071

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))

request = (
	"GET / HTTP/1.1\r\n"
	"Host: main2:8080\r\n"
	"User-Agent: PythonSocket/1.0\r\n"
	"Accept: */*\r\n"
	"Connection: close\r\n"
	"\r\n"
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
