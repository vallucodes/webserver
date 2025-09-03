#include "webserv.hpp"
#include "server/Server.hpp"

void	handle_sigint(int sig) {
	(void)sig;
	// close(server.server_fd); // maybe handle later
	printf("\nServer closed\n");
	exit(0);
}

int main()
{
	signal(SIGINT, handle_sigint);

	Server server;

	try {
		server.create();
		server.run();

	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}

//connect to server: telnet 127.0.0.1 <port>
