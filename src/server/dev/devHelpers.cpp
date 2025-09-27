#include "devHelpers.hpp"

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define CYAN "\033[1;36m"
#define YELLOW "\033[1;33m"
#define RESET "\033[0m"

// void printAllConfigGroups(std::vector<ListenerGroup>& groups) {
// 	for (size_t i = 0; i < groups.size(); ++i) {
// 		const ListenerGroup& group = groups[i];

// 		std::cout << std::endl << GREEN << "Listener Group #" << i << RESET << std::endl;

// 		// Print associated IP+Port for this group
// 		if (!group.configs.empty()) {
// 			uint32_t ip = group.configs[0].getAddress();
// 			int port = group.configs[0].getPort();
// 			struct in_addr addr;
// 			addr.s_addr = ip;
// 			std::cout << "IP: " << inet_ntoa(addr) << " | Port: " << port << std::endl;
// 		} else {
// 			std::cout << "No configs in this group!" << std::endl;
// 		}

// 		// Print default server if exists
// 		if (group.default_config)
// 			std::cout << "Default server: " << group.default_config->getName() << std::endl;
// 		else
// 			std::cout << "Default server: (none)" << std::endl;

// 		// Iterate through all server configs
// 		for (size_t j = 0; j < group.configs.size(); ++j) {
// 			const Server& srv = group.configs[j];
// 			std::cout << YELLOW << "\n  Server #" << j << " - " << srv.getName() << RESET << std::endl;
// 			std::cout << "    Root: " << srv.getRoot() << std::endl;
// 			std::cout << "    Index: " << srv.getIndex() << std::endl;
// 			std::cout << "    Max Body Size: " << srv.getMaxBodySize() << std::endl;

// 			// Error pages
// 			for (const auto& e : srv.getErrorPages())
// 				std::cout << "    Error page: " << e.first << " -> " << e.second << std::endl;

// 			// Locations
// 			for (const auto& loc : srv.getLocations()) {
// 				std::cout << CYAN << "    Location: " << loc.location << RESET << std::endl;
// 				std::cout << "      Allowed methods: ";
// 				for (const auto& m : loc.allowed_methods)
// 					std::cout << m << " ";
// 				std::cout << std::endl;
// 				std::cout << "      Index: " << loc.index << std::endl;
// 				std::cout << "      Autoindex: " << loc.autoindex << std::endl;
// 				std::cout << "      CGI Path: " << loc.cgi_path << std::endl;
// 				std::cout << "      CGI Ext: ";
// 				for (const auto& ext : loc.cgi_ext)
// 					std::cout << ext << " ";
// 				std::cout << std::endl;
// 				std::cout << "      Upload Path: " << loc.upload_path << std::endl;
// 			}
// 		}
// 	}
// 	std::cout << std::endl;
// }

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
