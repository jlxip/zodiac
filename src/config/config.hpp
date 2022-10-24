#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <arpa/inet.h>
#include <unordered_map>

struct CapsuleConfig {
	std::string name;
	std::string cert, key;

	std::string backend;
	uint16_t port;
	sockaddr_in saddr;

	size_t frontTimeout, backTimeout;
};

struct Config {
	std::string listenIP;
	uint16_t listenPort;
	size_t frontTimeout, backTimeout;
	//uint16_t workers = 3; // TODO

	std::unordered_map<std::string, CapsuleConfig> capsules;
	bool hasExplicitDefault = false;
	std::string def; // Default
};

Config parseConfig(const char* path = "zodiac.conf");

#endif
