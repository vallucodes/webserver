#include <gtest/gtest.h>
#include "../src/config/Config.hpp"

// Valid tests

// Test 1: Valid config, basic
TEST(ConfigValidationTest, ValidConfig1) {
	Config config;
	EXPECT_NO_THROW(config.validate("../test/unit/configs_for_testing/cfg1.conf"));
	// EXPECT_THROW(config.validate("/configs_for_testing/cfg1.conf"), std::exception);
}

// Test 2: Valid config, multiple locations
TEST(ConfigValidationTest, ValidConfig2) {
	Config config;
	EXPECT_NO_THROW(config.validate("../test/unit/configs_for_testing/cfg2.conf"));
}

// Test 3: Valid config, two servers, same IP+port
TEST(ConfigValidationTest, ValidConfig3) {
	Config config;
	EXPECT_NO_THROW(config.validate("../test/unit/configs_for_testing/cfg3.conf"));
}

// Test 4: Valid config, multiple servers, same and different IP+port
TEST(ConfigValidationTest, ValidConfig4) {
	Config config;
	EXPECT_NO_THROW(config.validate("../test/unit/configs_for_testing/cfg4.conf"));
}

// Test 5: Valid config, multiple servers, different IP+port
TEST(ConfigValidationTest, ValidConfig5) {
	Config config;
	EXPECT_NO_THROW(config.validate("../test/unit/configs_for_testing/cfg5.conf"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Failing tests

// Test 0: Invalid file
TEST(ConfigValidationTest, InvalidFile) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/non-existent");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: could not open config file.", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 1: Missing closing brace
TEST(ConfigValidationTest, MissingClosingBrace) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/missing_closing_brace.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Missing closing curly brace (syntax error)", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 2: Unbalanced closing brace
TEST(ConfigValidationTest, UnbalancedClosingBrace) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/unbalanced_brace.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string error_msg = e.what();
		EXPECT_TRUE(error_msg.find("Error: Config: Unbalanced }") != std::string::npos);
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 3: Invalid block type
TEST(ConfigValidationTest, InvalidBlockType) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/invalid_block_type.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string error_msg = e.what();
		EXPECT_TRUE(error_msg.find("Error: Config: Invalid block type") != std::string::npos);
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 4: Nested location blocks
TEST(ConfigValidationTest, NestedLocationBlocks) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/nested_location.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string error_msg = e.what();
		EXPECT_TRUE(error_msg.find("Error: Config: Nested 'location' block is not allowed") != std::string::npos);
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 5: Duplicate location
TEST(ConfigValidationTest, DuplicateLocation) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/duplicate_location.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string error_msg = e.what();
		EXPECT_TRUE(error_msg.find("Error: Config: Duplicate location") != std::string::npos);
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 6: Invalid location value
TEST(ConfigValidationTest, InvalidLocationValue) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/invalid_location_value.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Invalid value for directive: location", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 7: Server block not top-level
TEST(ConfigValidationTest, ServerBlockNotTopLevel) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/nested_server.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string error_msg = e.what();
		EXPECT_TRUE(error_msg.find("Error: Config: 'server' block must be top-level only") != std::string::npos);
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 8: Missing directory type location
TEST(ConfigValidationTest, MissingDirectoryLocation) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/no_directory_location.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Missing directory type of location", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 9: Keyword before block
TEST(ConfigValidationTest, KeywordBeforeBlock) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/keyword_before_block.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string error_msg = e.what();
		EXPECT_TRUE(error_msg.find("Error: Config: Keyword outside of any block") != std::string::npos);
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 10: Invalid port value
TEST(ConfigValidationTest, InvalidPortValue) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/invalid_port.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Invalid value for directive: listen", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 11: Invalid IP address
TEST(ConfigValidationTest, InvalidIPAddress) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/invalid_ip.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Invalid value for directive: host", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 12: Invalid index file (not .html)
TEST(ConfigValidationTest, InvalidIndexFile) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/invalid_index.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Invalid value for directive: index", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 13: Invalid max body size
TEST(ConfigValidationTest, InvalidMaxBodySize) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/invalid_max_body_size.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string msg = e.what();
		EXPECT_TRUE(std::regex_search(msg, std::regex("client_max_body_size\\s*-1")));
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 14: Invalid error page (mismatched numbers)
TEST(ConfigValidationTest, InvalidErrorPage) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/invalid_error_page.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Invalid value for directive: error_page", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 15: Invalid HTTP methods
TEST(ConfigValidationTest, InvalidHTTPMethods) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/invalid_methods.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Invalid value for directive: allow_methods", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 16: Invalid CGI extensions
TEST(ConfigValidationTest, InvalidCGIExtensions) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/invalid_cgi_ext.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Invalid value for directive: cgi_ext", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 17: Invalid autoindex value
TEST(ConfigValidationTest, InvalidAutoindexValue) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/invalid_autoindex.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Invalid value for directive: autoindex", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 18: Repeated directive
TEST(ConfigValidationTest, RepeatedDirective) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/duplicate_directive_server.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string error_msg = e.what();
		EXPECT_TRUE(error_msg.find("Error: Config: Repeated directive") != std::string::npos);
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 19: Malformed directive
TEST(ConfigValidationTest, MalformedDirective) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/malformed_directive.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string error_msg = e.what();
		EXPECT_TRUE(error_msg.find("Error: Config: Malformed directive") != std::string::npos);
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 20: Missing mandatory server directive - listen
TEST(ConfigValidationTest, MissingMandatoryListen) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/missing_listen.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Missing mandatory server directory: listen", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 21: Missing mandatory server directive - server_name
TEST(ConfigValidationTest, MissingMandatoryServerName) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/missing_server_name.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Missing mandatory server directory: server_name", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 22: Missing mandatory server directive - host
TEST(ConfigValidationTest, MissingMandatoryHost) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/missing_host.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Missing mandatory server directory: host", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 23: Missing mandatory server directive - root
TEST(ConfigValidationTest, MissingMandatoryRoot) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/missing_root.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Missing mandatory server directory: root", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 24: Missing mandatory location directive - allow_methods (directory)
TEST(ConfigValidationTest, MissingMandatoryAllowMethods) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/missing_allow_methods.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Missing mandatory location directory: allow_methods", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 25: Missing mandatory location directive - index (directory)
TEST(ConfigValidationTest, MissingMandatoryIndex) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/missing_index.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Missing mandatory location directory: index", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 26: Missing mandatory CGI directive - cgi_path
TEST(ConfigValidationTest, MissingMandatoryCGIPath) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/missing_cgi_path.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Missing mandatory location directory: cgi_path", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 27: Missing mandatory CGI directive - cgi_ext
TEST(ConfigValidationTest, MissingMandatoryCGIExt) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/missing_cgi_ext.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Missing mandatory location directory: cgi_ext", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 28: Port out of allowed range (too low)
TEST(ConfigValidationTest, PortTooLow) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/port_too_low.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Invalid value for directive: listen", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 29: Port out of allowed range (too high)
TEST(ConfigValidationTest, PortTooHigh) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/port_too_high.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Invalid value for directive: listen", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 30: Restricted port
TEST(ConfigValidationTest, RestrictedPort) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/restricted_port.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Invalid value for directive: listen", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 31: Duplicate directive in location
TEST(ConfigValidationTest, DuplicateDirectiveLocation) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/duplicate_directive_location.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string error_msg = e.what();
		EXPECT_TRUE(error_msg.find("Error: Config: Repeated directive:") != std::string::npos);
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 31: Duplicate directive in cgi
TEST(ConfigValidationTest, DuplicateDirectiveCgi) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/duplicate_directive_cgi.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string error_msg = e.what();
		EXPECT_TRUE(error_msg.find("Error: Config: Repeated directive:") != std::string::npos);
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 32: Keyword after block
TEST(ConfigValidationTest, KeywordAfterBlock) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/keyword_after_block.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string error_msg = e.what();
		EXPECT_TRUE(error_msg.find("Error: Config: Keyword outside of any block") != std::string::npos);
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 32: Keyword after block
TEST(ConfigValidationTest, MissingDirectiveMultipleServers) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/missing_directive_multiple_servers.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string error_msg = e.what();
		EXPECT_TRUE(error_msg.find("Error: Config: Missing mandatory server directory:") != std::string::npos);
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 33: Malformed directive2
TEST(ConfigValidationTest, MalformedDirective2) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/malformed_directive2.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string error_msg = e.what();
		EXPECT_TRUE(error_msg.find("Error: Config: Malformed directive") != std::string::npos);
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 34: Missing body for location block
TEST(ConfigValidationTest, MissingBody) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/missing_body.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string error_msg = e.what();
		EXPECT_TRUE(error_msg.find("Error: Config: Missing mandatory location directory:") != std::string::npos);
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 35: Too many HTTP mthods in location block
TEST(ConfigValidationTest, TooManyHTTPMethods) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/too_many_HTTP_methods.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		std::string error_msg = e.what();
		EXPECT_TRUE(error_msg.find("Error: Config: Malformed directive") != std::string::npos);
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}

// Test 34: Missing location block
TEST(ConfigValidationTest, MissingLocation) {
	Config config;
	try {
		config.validate("../test/unit/configs_for_testing/missing_location.conf");
		FAIL() << "Expected std::exception to be thrown";
	} catch (const std::exception& e) {
		EXPECT_STREQ("Error: Config: Missing directory type of location", e.what());
	} catch (...) {
		FAIL() << "Expected std::exception, got a different exception";
	}
}
