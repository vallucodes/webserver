#include "Server.hpp"

void	Server::create() {
	server_fd = socket(AF_INET, SOCK_STREAM, 0); // create TCP socket that can talk over IPv4.
	if (server_fd < 0)
		throw std::runtime_error("Error: Socket creation");

	srand(time(0));
	int port = 1024 + rand() % (10000 - 1024 + 1);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;			// Use internet protocol IPv4
	addr.sin_port = htons(port);		// set port to listen to
	addr.sin_addr.s_addr = INADDR_ANY;	// listen to all availabe interfaces

	if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {	// bind IP address to a existing socket
		close (server_fd);
		throw std::runtime_error("Error: bind");
	}
	if (listen(server_fd, SOMAXCONN) < 0)	//give socket ability to receive connections
		throw std::runtime_error("Error: listen");

	std::cout << "Server listening on port " << port << "\n";
}

void	Server::run() {
	while (true)
	{
		int client_fd = accept(server_fd, nullptr, nullptr); // 2nd argument: collect clients IP and port. 3rd argument tells size of the buffer of second argument
		if (client_fd < 0)
			throw std::runtime_error("Error: accept");

		std::cout << "Client connected\n" << std::endl;

		char buffer[1024];
		int bytes;
		while ((bytes = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {	// send response when something is received
			std::string response = "Nice talking to you!\n";
			send(client_fd, response.c_str(), response.size(), 0);
		}
		close(client_fd);
		std::cout << "Client disconnected\n";
	}
	close(server_fd);
}
