#include "devHelpers.hpp"

void	printAllConfigs(std::vector<Server> cfg) {
	for (size_t i = 0; i < cfg.size(); ++i)
	{
		std::cout << std::endl << YELLOW << "Server settings" << RESET << std::endl;
		std::cout << "Port: " << cfg[i].getPort() << std::endl;
		uint32_t ip = cfg[i].getAddress(); // network byte order
		struct in_addr addr;
		addr.s_addr = ip;
		std::cout << "Address: " << inet_ntoa(addr) << std::endl;
		std::cout << "Client max body size: " << cfg[i].getMaxBodySize() << std::endl;
		std::cout << "Name: " << cfg[i].getName() << std::endl;
		std::cout << "Root: " << cfg[i].getRoot() << std::endl;
		std::cout << "Index: " << cfg[i].getIndex() << std::endl;
		for (const auto& entry : cfg[i].getErrorPages())
			std::cout << "Error page: " << entry.first << " -> " << entry.second << std::endl;
		for (const auto& entry : cfg[i].getLocations())
		{
			std::cout << CYAN << "Location: " << entry.location << RESET << std::endl;
			std::cout << "      Allowed methods: ";
			for (const auto& entry : entry.allowed_methods)
				std::cout << entry << " ";
			std::cout << std::endl << "      Index(loc): " << entry.index << std::endl;
			std::cout << "      Autoindex: " << entry.autoindex << std::endl;
			std::cout << "      CGI Path: " << entry.cgi_path << std::endl;
			std::cout << "      CGI ext: ";
			for (const auto& entry : entry.cgi_ext)
				std::cout << entry << std::endl;
			std::cout << "      Upload path: " << entry.upload_path << std::endl;
		}
	}
	std::cout << std::endl;
}

void	printServerConfig(Server conf) {
	std::cout << std::endl << YELLOW << "Server settings" << RESET << std::endl;
	std::cout << "Port: " << conf.getPort() << std::endl;
	struct in_addr addr;
	addr.s_addr = conf.getAddress();
	std::cout << "Address: " << inet_ntoa(addr) << std::endl;
	std::cout << "Client max body size: " << conf.getMaxBodySize() << std::endl;
	std::cout << "Name: " << conf.getName() << std::endl;
}
