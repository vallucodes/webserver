import socket

# Server configuration
HOST = '127.0.0.1'
PORT = 8050

# Define the chunks you want to send
chunks = [
    "Hello, ",
    "world! ",
    "This is a test."
]

# Build the raw HTTP request headers
request_headers = (
    "POST / HTTP/1.1\r\n"
    f"Host: {HOST}:{PORT}\r\n"
    "Transfer-Encoding: chunked\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
)

# Connect to the server
with socket.create_connection((HOST, PORT)) as sock:
    # Send headers
    sock.sendall(request_headers.encode())

    # Send each chunk
    for chunk in chunks:
        size = f"{len(chunk):X}"  # chunk size in hex
        sock.sendall(f"{size}\r\n".encode())
        sock.sendall(chunk.encode())
        sock.sendall(b"\r\n")

    # Send final zero-length chunk to indicate end
    sock.sendall(b"0\r\n\r\n")

    # Optionally, receive response
    response = sock.recv(4096)
    print(response.decode())

