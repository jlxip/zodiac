#ifndef COMMON_HPP
#define COMMON_HPP

#include <tls/tls.hpp>
#include <config/config.hpp>
#include <vector>
#include <ssockets.h>

// After initialization, these two turn read-only
extern Config globalConfig;
extern TLS::Server globalServer;

struct Data {
	TLS::Connection conn;
	bool doomed = false;
	CapsuleConfig* capsule = nullptr;
	int frontend = 0;
	int backend = 0;
	bool noTimeout = true;

	char* buffer = nullptr;
	size_t bufferSize = 0;
	size_t ctr = 0;

	// All bytes written in buffer in the past from backOut
	// This is just to return an error if response was empty
	size_t fullctr = 0;
};

// aux.cpp
std::vector<std::string> commaSplit(const char* str);

#endif
