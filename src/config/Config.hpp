#pragma once
#include <iostream>
#include <vector>
#include <unordered_set>
#include <regex>
#include <fstream>
#include <string>

#include "../server/Server.hpp"
#include "ConfigValidator.hpp"
#include "ConfigExtractor.hpp"

class Config {

	private:
		ConfigValidator	validator;
		ConfigExtractor	parser;

	public:
		void				validate(const std::string& config);
		std::vector<Server>	parse(const std::string& config);
};
