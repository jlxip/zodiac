#include "config.hpp"
#include <iostream>
#include <iniparser.h>
#include <unistd.h>
#include <netdb.h>
#include <vector>

std::vector<std::string> commaSplit(const char* str) {
	std::vector<std::string> ret;

	std::string aux;
	for(size_t i=0; str[i]; ++i) {
		char c = str[i];
		if(c == ',') {
			ret.push_back(aux);
			aux.clear();
		} else if(c != ' ') {
			// Ignore all spaces
			aux += c;
		}
	}

	if(aux.size())
		ret.push_back(aux);
	return ret;
}

Config parseConfig(const char* path) {
	Config ret;
	auto* dict = iniparser_load(path);

	// Get global options
	ret.listenIP = iniparser_getstring(dict, "zodiac:listenIP", "0.0.0.0");
	ret.listenPort = iniparser_getint(dict, "zodiac:listenPort", 1965);
	ret.frontTimeout = iniparser_getint(dict, "zodiac:frontTimeout", 5);
	ret.backTimeout = iniparser_getint(dict, "zodiac:backTimeout", 5);

	// Get enabled capsules
	const char* rawenabled = iniparser_getstring(dict, "zodiac:enabled", "");
	if(!rawenabled) {
		std::cerr
			<< "No \"enabled\" key in the \"zodiac\" section in the config file."
			<< std::endl;
		exit(EXIT_FAILURE);
	}

	auto enabled = commaSplit(rawenabled);

	// For each capsule, get its info
	for(auto const& x : enabled) {
		CapsuleConfig s;
		s.name = iniparser_getstring(dict, (x+":name").c_str(), "");
		s.cert = iniparser_getstring(dict, (x+":cert").c_str(), "cert.pem");
		s.key = iniparser_getstring(dict, (x+":key").c_str(), "key.pem");
		s.backend = iniparser_getstring(dict, (x+":backend").c_str(), "localhost");
		s.port = iniparser_getint(dict, (x+":port").c_str(), 0);
		s.frontTimeout = iniparser_getint(dict, (x+":frontTimeout").c_str(), 0);
		s.backTimeout = iniparser_getint(dict, (x+":backTimeout").c_str(), 0);

		// Is it default?
		bool isDefault = iniparser_getboolean(dict, (x+":default").c_str(), 0);
		if(isDefault) {
			if(ret.def.size()) {
				std::cerr
					<< "\"" << x << "\" is marked as default, "
					<< "but so is \"" << ret.def << "\"."
					<< std::endl;
				exit(EXIT_FAILURE);
			}

			ret.def = x;
			ret.hasExplicitDefault = true;
		}

		// Check fields
		if(!s.name.size() && !isDefault) {
			std::cerr
				<< "No \"name\" key for capsule \"" << x << "\"."
				<< std::endl;
			exit(EXIT_FAILURE);
		}

		// TODO check files for prettier error

		if(!s.port) {
			std::cerr
				<< "No \"port\" key for capsule \"" << x << "\"."
				<< std::endl;
			exit(EXIT_FAILURE);
		}

		// Resolve backend field
		hostent* he = gethostbyname(s.backend.c_str());
		if(!he) {
			std::cerr << "Could not resolve: " << s.backend << std::endl;
			exit(EXIT_FAILURE);
		}

		// Set values for easy access later
		sockaddr_in saddr;
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(s.port);
		saddr.sin_addr = *((in_addr*)he->h_addr);
		memset(saddr.sin_zero, 0, 8);
		s.saddr = saddr;

		// Add to global config now
		ret.capsules[x] = s;
	}

	if(!ret.def.size()) {
		// No default capsule, pick the first one (unordered)
		ret.def = (*(ret.capsules.begin())).first;
	}

	iniparser_freedict(dict);
	return ret;
}
