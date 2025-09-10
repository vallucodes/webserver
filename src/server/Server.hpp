#pragma once
#include <vector>
#include <poll.h>
#include <map>
#include "webserv.hpp"

struct Location
{
	std::vector<std::string>	allowed_methods;
	std::string					index;
	bool						autoindex;
	std::string					cgi_path;
	std::string					cgi_ext;
	std::string					default_file;
	std::string					upload_path;
};

class Server {

	private:
		uint32_t					_address;
		int							_port;
		std::string					_name;
		std::string					_root;
		std::map<int, std::string>	_error_pages;
		size_t						_client_max_body_size;
		std::vector<Location>		_locations;

	public:
		Server() = default;
		~Server() = default;

		int	create();

		void	setAddress(uint32_t address);
		void	setPort(int port);
		void	setMaxBodySize(int max_body_size);
		void	setName(const std::string& name);

		uint32_t			getAddress() const;
		int					getPort() const;
		int					getMaxBodySize() const;
		const std::string&	getName() const;
};
