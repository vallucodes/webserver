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

	fds.push_back({server_fd, POLLIN, 0});

	std::cout << "Server listening on port " << port << "\n";
}

void	Server::run() {
	while (true)
	{
		if(poll(fds.data(), fds.size(), POLL_TIME_OUT) < 0);
			throw std::runtime_error("Error: poll");

		for (size_t i = 0; i < fds.size(); ++i) {
			if (fds[i].revents & POLLIN) {
				if (fds[i].fd == server_fd) {
					// new client
					sockaddr_in client_addr{};
					socklen_t addrlen = sizeof(client_addr);
					int client_fd = accept(server_fd, (sockaddr*)&client_addr, &addrlen); // 2nd argument: collect clients IP and port. 3rd argument tells size of the buffer of second argument
					if (client_fd >= 0) {
						std::cout << "New client connected: "
								<< inet_ntoa(client_addr.sin_addr) << ":"
								<< ntohs(client_addr.sin_port) << "\n";
						fds.push_back({client_fd, POLLIN, 0});
					}
					else
						throw std::runtime_error("Error: accept");
				}
				else {
				// Existing client sending data
				char buffer[1024];
				int bytes = recv(fds[i].fd, buffer, sizeof(buffer), 0);
				if (bytes <= 0) {
					std::cout << "Client disconnected\n";
					close (fds[i].fd);
					fds.erase(fds.begin() + i);
					--i;
				}
				else
					send(fds[i].fd, buffer, bytes, 0);
				}
			}
		}
	}
	close(server_fd);
}
