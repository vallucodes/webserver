#pragma once
#include <vector>
#include <poll.h>
#include "webserv.hpp"

struct Location
{
	std::string	allow_methods;
	std::string	index;
	bool		autoindex;
	std::string	cgi_path;
	std::string	cgi_ext;
	std::string	default_file;
	std::string	upload_path;
};
class Server {

	private:
		uint32_t					_address;
		int							_port;
		std::string					name;
		std::string					address;
		int							port;
		std::string					root;
		std::map<int, std::string>	error_pages;
		size_t						client_max_body_size;
		std::vector<Location>		locations;

	public:
		Server(uint32_t address, int port);
		~Server() = default;

		int	create();
};
