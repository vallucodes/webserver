#pragma once
#include <iostream>
#include <vector>
#include <unordered_set>
#include <set>
#include <regex>
#include <fstream>
#include <string>

#include "../server/Server.hpp"

enum LocationType {
	NONE,
	DIRECTORY,
	FILE_EXTENSION
};

struct Directive {
	std::string name;
	std::regex	pattern;
	std::function<bool(const std::string&)> valueChecker;
	bool isSet = false;
	LocationType location_type = NONE;
};

class ConfigValidator {

	private:

		std::unordered_set<std::string>		_mandatory_server_directives;
		std::unordered_set<std::string>		_mandatory_location_directives_directory;
		std::unordered_set<std::string>		_mandatory_location_directives_cgi;
		std::vector<Directive>				_server_directives;
		std::vector<Directive>				_location_directives;
		static std::vector<std::string>		_methods;
		static std::vector<std::string>		_cgi_extensions;

		void		validateKeyword(const std::string& line, const std::string& context);
		void		handleOpenBlock(std::stack<std::string>& blockstack, const std::string& line, LocationType& current_type, bool& location_present, std::set<std::string>& locations);
		void		handleCloseBlock(std::stack<std::string>& blockstack, const std::string& line, LocationType current_type, bool location_present);
		void		handleKeyword(std::stack<std::string>&	blockstack, const std::string& line);

		static bool	validatePort(const std::string& line);
		static bool	validateIP(const std::string& line);
		static bool	validateIndex(const std::string& line);
		static bool	validateMaxBodySize(const std::string& line);
		static bool	validateErrorPage(const std::string& line);
		bool		validateLocation(const std::string& line, LocationType& type, bool& location_present);
		static bool	validateMethods(const std::string& line);
		static bool	validateExt(const std::string& line);
		static bool	validateAutoindex(const std::string& line);

		void		resetDirectivesFlags(const std::string& blocktype);
		void		verifyMandatoryDirectives(const std::string& blocktype, LocationType current);

	public:

		void		validateFields(std::ifstream& cfg);

		ConfigValidator ();
		~ConfigValidator () = default;
};
