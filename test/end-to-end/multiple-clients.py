import socket
import threading

SERVER_HOST = '127.0.0.1'	# Change to your server IP
SERVER_PORT = 1135			# Change to your server port
NUM_CLIENTS = 300			# Number of clients to create

def client_task(id):
	try:
		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		sock.connect((SERVER_HOST, SERVER_PORT))
		print(f"Client {id} attempted to connect")

		# Example: send a message
		# request = f"Hello from client {id}"
		# sock.sendall(request.encode())

		# Receive response
		sock.settimeout(10)  # wait max 5 seconds
		try:
			response = sock.recv(1024)
			print(f"Client {id} received: {response.decode()}")
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
