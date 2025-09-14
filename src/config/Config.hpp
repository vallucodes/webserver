#pragma once
#include <iostream>
#include <vector>
#include <regex>
#include <fstream>
#include <string>

#include "../server/Server.hpp"

struct Directive {
	std::string name;
	std::regex	pattern;
	std::function<bool(const std::string&)> valueChecker;
	bool isSet = false;
};

class Config {

	private:

		std::vector<Directive> _server_directives;
		std::vector<Directive> _location_directives;

		void		checkKeywords(const std::string& line, const std::string& context);

		static bool	validatePort(const std::string& line);
		static bool	validateIP(const std::string& line);
		static bool	validateIndex(const std::string& line);
		static bool	validateMaxBodySize(const std::string& line);
		static bool	validateErrorPage(const std::string& line);

		static void	extractServerFields(std::vector<Server>& servs, std::ifstream& cfg);
		static void	extractLocationFields(Server& serv, Location& loc, std::ifstream& cfg);

		static void	extractPort(Server& serv, const std::string& line);
		static void	extractAddress(Server& serv, const std::string& line);
		static void	extractMaxBodySize(Server& serv, const std::string& line);
		static void	extractName(Server& serv, const std::string& line);
		static void	extractRoot(Server& serv, const std::string& line);
		static void	extractIndex(Server& serv, const std::string& line);
		static void	extractErrorPage(Server& serv, const std::string& line);

		static void	extractLocation(Location& loc, const std::string& line);
		static void	extractAllowedMethods(Location& loc, const std::string& line);
		static void	extractIndexLoc(Location& loc, const std::string& line);
		static void	extractAutoindex(Location& loc, const std::string& line);
		static void	extractCgiPath(Location& loc, const std::string& line);
		static void	extractCgiExt(Location& loc, const std::string& line);
		static void	extractUploadPath(Location& loc, const std::string& line);

		void		resetDirectivesFlags();

	public:
		void				validate(const std::string& config);
		std::vector<Server>	parse(const std::string& config);
		Config ();
		~Config () = default;
};
