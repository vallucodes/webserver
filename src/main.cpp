#include "webserv.hpp"
#include "server/Server.hpp"
#include "server/Cluster.hpp"

int main(int ac, char **av)
{
	try {
		if (ac != 2 && ac != 1)
			throw std::runtime_error("Error: Incorrect amount of arguments. Usage: ./webserv <path_to_config>");
		Cluster cluster;
		if (ac == 2)
			cluster.config(av[1]);
		else if (ac == 1)
			cluster.config(DEFAULT_CONF);
		cluster.create();
		cluster.run();
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}

// setting up same port for different serv should not be allowed
