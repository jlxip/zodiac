#include "config.hpp"
#include <iostream>
#include <iniparser.h>
#include <unistd.h>
#include <netdb.h>

Config parseConfig(const char* path) {
	Config ret;
	auto* dict = iniparser_load(path);

	ret.server.cert = iniparser_getstring(dict, "server:cert", "cert.pem");
	ret.server.key = iniparser_getstring(dict, "server:key", "key.pem");
	ret.server.server = iniparser_getstring(dict, "server:server", "localhost");
	ret.server.port = iniparser_getint(dict, "server:port", 0);

	// TODO check files for prettier error
	if(!ret.server.port) {
		std::cerr
			<<"No \"port\" key (backend) found in configuration file."
			<< std::endl;
		exit(EXIT_FAILURE);
	}

	// Resolve server
	hostent* he = gethostbyname(ret.server.server.c_str());
	if(!he) {
		std::cerr << "Could not resolve: " << ret.server.server << std::endl;
		exit(EXIT_FAILURE);
	}

	// Set values for easy access later
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(ret.server.port);
	saddr.sin_addr = *((in_addr*)he->h_addr);
	memset(saddr.sin_zero, 0, 8);
	ret.server.saddr = saddr;

	iniparser_freedict(dict);
	return ret;
}
