import socket
import threading

SERVER_HOST = '127.0.0.1'	# Change to your server IP
SERVER_PORT = 1703			# Change to your server port
NUM_CLIENTS = 10			# Number of clients to create

def client_task(id):
	try:
		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		sock.connect((SERVER_HOST, SERVER_PORT))
		print(f"Client {id} attempted to connect")

		body = "123456789102134256"
		request = (
			f"POST / HTTP/1.1\r\n"
			f"Host: {SERVER_HOST}\r\n"
			f"Content-Length: {len(body)}\r\n"
			"Content-Type: text/plain\r\n"
			"\r\n"
			f"{body}"
		)
		sock.sendall(request.encode())

		# Receive response
		sock.settimeout(10)
		try:
		while True:
			data = sock.recv(1024)
			if not data:
				break  # server closed connection
			print(f"Client {id} received: {data.decode()}")
		except socket.timeout:
			print(f"Client {id} timed out waiting for response")

		sock.close()
	except Exception as e:
		print(f"Client {id} error: {e}")

threads = []
for i in range(NUM_CLIENTS):
	t = threading.Thread(target=client_task, args=(i,))
	t.start()
	threads.append(t)

for t in threads:
	t.join()

print("All clients finished")
