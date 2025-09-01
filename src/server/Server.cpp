#include "Server.hpp"

int Server::create() {
	int server_fd = socket(AF_INET, SOCK_STREAM, 0); //create TCP socket that can talk over IPv4.
	if (server_fd < 0)
	{
		perror("socket");
		return 1;
	}
	std::cout << server_fd << std::endl;

	srand(time(0));
	int port = 1024 + rand() % (10000 - 1024 + 1);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;			//Use internet protocol IPv4
	addr.sin_port = htons(port);		//set port to listen to
	addr.sin_addr.s_addr = INADDR_ANY;	//listen to all availabe interfaces

	if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)		//bind IP address to a existing socket
	{
		perror("bind");
		return 1;
	}
	if (listen(server_fd, SOMAXCONN) < 0)
	{
		perror("listen");
		return 1;
	}

	std::cout << "Server listening on port " << port << "\n";

	return server_fd;
}
