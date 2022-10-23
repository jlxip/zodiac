#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <arpa/inet.h>
//#include <unordered_map>

struct ServerConfig {
	std::string cert, key;

	std::string server;
	uint16_t port;
	sockaddr_in saddr;
};

struct Config {
	//uint32_t ip = 0; // Where to listen, TODO
	//uint16_t port = 1965;
	//uint16_t workers = 3; // TODO

	ServerConfig server; // This will be multiple soon
};

Config parseConfig(const char* path = "zodiac.conf");

#endif
