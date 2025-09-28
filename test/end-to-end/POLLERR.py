import socket, os, struct

HOST = "127.0.0.1"
PORT = 8082

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))

# Send some data
sock.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")

# Abruptly terminate the connection (RST)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack('ii', 1, 0))
sock.close()
