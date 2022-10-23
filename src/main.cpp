#include <iostream>
#include <common.hpp>
#include <unistd.h>

// These will be in config soon
#define DEFAULT_PORT 1965
#define FRONTEND_TIMEOUT 5
#define BACKEND_TIMEOUT 5

#define BUFFER_SIZE 1024

Config globalConfig;
TLS::Server globalServer;

int main() {
	std::cout << "Zodiac starting. Parsing configuration..." << std::endl;
	globalConfig = parseConfig();
	globalServer = {DEFAULT_PORT};

	std::cout << "Setting up capsules..." << std::endl;
	for(auto const& x : globalConfig.capsules) {
		std::cout << "-> Enabling " << x.first << std::endl;
		auto* ctx = TLS::createContext
			(x.second.cert.c_str(), x.second.key.c_str());
		globalServer.ctxs[x.first] = ctx;
		globalServer.names[x.second.name] = x.first;
	}
	globalServer.defaultContext = globalServer.ctxs[globalConfig.def];

	globalServer.setTimeout(FRONTEND_TIMEOUT);
	std::cout << "Listening on port " << DEFAULT_PORT << std::endl;
	std::cout << "Ready to go" << std::endl;

	// Handle requests
	char buffer[BUFFER_SIZE];
	while(true) {
		auto conn = globalServer.acc();

		// Get path. It must be done before sending anything
		auto line = conn.recvl();
		if(!line.size()) {
			conn.cl(); continue;
		}

		// Is server name known?
		std::string capsuleName;
		auto it = globalServer.names.find(conn.getSN());
		if(it == globalServer.names.end()) {
			// Unknown. Is default explicitly set?
			if(globalConfig.hasExplicitDefault) {
				// Yep, no worries
				capsuleName = globalConfig.def;
			} else {
				// No; reject the request
				conn.send("51 zodiac: unrecognized server name\r\n");
				conn.cl(); continue;
			}
		} else {
			// Nice
			capsuleName = (*it).second;
		}

		auto& capsule = globalConfig.capsules[capsuleName];

		// Connect to backend
		int backend = socket(AF_INET, SOCK_STREAM, 0);
		if(backend < 0) {
			conn.send("43 zodiac: crowded\r\n");
			conn.cl(); continue;
		}
		if(connect(backend, (struct sockaddr*)&(capsule.saddr),
			       sizeof(sockaddr)) < 0) {
			conn.send("43 zodiac: could not connect to backend\r\n");
			conn.cl(); continue;
		}

		// Send line and client's IP
		send(backend, line.c_str(), line.size(), 0);
		send(backend, "\r\n", 2, 0);
		send(backend, conn.ip.c_str(), conn.ip.size(), 0);
		send(backend, "\r\n", 2, 0);

		// Backend timeout
		timeval timeout;
		timeout.tv_sec = BACKEND_TIMEOUT;
		timeout.tv_usec = 0;
		setsockopt(backend, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

		// Get response
		while(true) {
			int got = recv(backend, buffer, BUFFER_SIZE, 0);
			if(!got)
				break; // That's it

			if(got == -1) {
				conn.send("43 zodiac: backend timed out\r\n");
				break;
			}

			conn.send(buffer, got);
		}

		close(backend);
		conn.cl();
	}
}
