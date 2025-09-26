#include "webserv.hpp"
#include "server/Server.hpp"
#include "server/Cluster.hpp"

int main(int ac, char **av)
{
	try {
		if (ac != 2)
			throw std::runtime_error("Error: Incorrect amount of arguments. Usage: ./webserv <path_to_config>");
		Cluster cluster;
		cluster.config(av[1]);
		cluster.create();
		cluster.run();
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}
