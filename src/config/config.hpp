#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <arpa/inet.h>
#include <unordered_map>
#include <vector>
#include <mutex>

struct CapsuleConfig {
	std::string name;
	std::string cert, key;

	std::vector<sockaddr_in> addrs;
	std::mutex addrLock;
	size_t addr = 0;

	size_t frontTimeout, backTimeout;

	void operator=(CapsuleConfig&& other) {
		name = std::move(other.name);
		cert = std::move(other.cert);
		key = std::move(other.key);
		addrs = std::move(other.addrs);
		addr = std::move(other.addr);
		frontTimeout = std::move(frontTimeout);
		backTimeout = std::move(backTimeout);
	}
};

struct Config {
	std::string listenIP;
	uint16_t listenPort;
	size_t frontTimeout, backTimeout, hsTimeout;
	size_t workers;

	std::unordered_map<std::string, CapsuleConfig> capsules;
	bool hasExplicitDefault = false;
	std::string def; // Default
};

Config parseConfig(const char* path);

#endif
