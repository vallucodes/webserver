#include "devHelpers.hpp"

void	printAllConfigs(std::vector<Server> cfg) {
	for (size_t i = 0; i < cfg.size(); ++i)
	{
		std::cout << "Port: " << cfg[i].getPort() << std::endl;
		uint32_t ip = cfg[i].getAddress(); // network byte order
		struct in_addr addr;
		addr.s_addr = ip;
		std::cout << "Address: " << inet_ntoa(addr) << std::endl;
		std::cout << "Client max body size: " << cfg[i].getMaxBodySize() << std::endl;
		std::cout << "Name: " << cfg[i].getName() << std::endl;
	}
}
