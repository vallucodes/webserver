#pragma once
#include <vector>
#include <poll.h>
#include <map>
#include "webserv.hpp"

struct Location
{
	std::string					location;
	std::vector<std::string>	allowed_methods;
	std::string					index;
	bool						autoindex;
	std::string					cgi_path;
	std::vector<std::string>	cgi_ext;
	std::string					upload_path;
	std::string					return_url;
};

class Server {

	private:
		uint32_t					_address;
		int							_port;
		std::string					_name;
		std::string					_root;
		std::string					_index;
		std::map<int, std::string>	_error_pages;
		size_t						_client_max_body_size = MAX_BODY_SIZE;
		std::vector<Location>		_locations;

	public:
		Server() = default;
		~Server() = default;

		int	create();

		void	setAddress(uint32_t address);
		void	setPort(int port);
		void	setMaxBodySize(int max_body_size);
		void	setName(const std::string& name);
		void	setRoot(const std::string& root);
		void	setIndex(const std::string& index);
		void	setErrorPage(int error_index, const std::string& page);

		void	setLocation(Location loc);

		uint32_t			getAddress() const;
		int					getPort() const;
		int					getMaxBodySize() const;
		const std::string&	getName() const;
		const std::string&	getRoot() const;
		const std::string&	getIndex() const;
		const std::map<int, std::string>&	getErrorPages() const;
		const std::vector<Location>&		getLocations() const;
};
