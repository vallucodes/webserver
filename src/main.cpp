#include "webserv.hpp"
#include "server/Server.hpp"

int server_fd;

void	handle_sigint(int sig) {
	(void)sig;
	close(server_fd);
	printf("\nServer closed\n");
	exit(0);
}

int main()
{
	signal(SIGINT, handle_sigint);

	Server server;

	int server_fd = server.create();

	while (true)
	{
		int client_fd = accept(server_fd, nullptr, nullptr);
		if (client_fd < 0)
		{
			perror("accept");
			continue ;
		}

		std::cout << "Client connected\n" << std::endl;

		char buffer[1024];
		int bytes;
		while ((bytes = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
			// Echo back what was received
			send(client_fd, buffer, bytes, 0);
		}
		close(client_fd);
		std::cout << "Client disconnected\n";
	}
	close(server_fd);
	return 0;
}
