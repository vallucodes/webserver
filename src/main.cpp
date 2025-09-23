#include "webserv.hpp"
#include "server/Server.hpp"
#include "server/Cluster.hpp"

void	handle_sigint(int sig) {
	(void)sig;
	// close(server.server_fd); // maybe handle later
	std::cout << RED << "\n" << time_now() << "	Server closed\n" << RESET;
	exit(0);
}

int main(int ac, char **av)
{
	signal(SIGINT, handle_sigint);

	// if (ac != 2)
	// 	return 1;
	(void)ac;

	Cluster cluster;
	try {
		cluster.config(av[1]);
		cluster.create();
		cluster.run();

	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}

// connect to server: telnet 127.0.0.1 <port>

// empty request is received from client and sent to parsing after successful chunked request
// handle 2,5 requests
// check chunked body size thing, is it handled correctly
// if request is big, save to tmp file
