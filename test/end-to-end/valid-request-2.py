import socket

HOST = "127.0.0.1"
PORT = 8082

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))

# Two GET requests concatenated
requests = (
	"GET /uploads HTTP/1.1\r\n"
	"Host: localhost\r\n"
	"Connection: keep-alive\r\n"
	"\r\n"
	"GET /uploads HTTP/1.1\r\n"
	"Host: localhost\r\n"
	"Connection: keep-alive\r\n"
	"\r\n"
)

sock.sendall(requests.encode())

# Read and print responses as they arrive
print("Start of waiting for responses")
response = b""
sock.settimeout(10)
try:
	while True:
		data = sock.recv(8192)
		if not data:
			break
		print(f"Client received chunk:\n{data.decode()}")
		response += data
except socket.timeout:
	print(f"Client {id} timed out waiting for response")
	sock.close()

# print("\n=== Full response(s) ===")
# print(response.decode())

sock.close()
