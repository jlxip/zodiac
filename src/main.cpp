#include <iostream>
#include <common.hpp>
#include <unistd.h>

// This will be in config soon
#define DEFAULT_PORT 1965

#define BUFFER_SIZE 1024

int main() {
	std::cout << "Zodiac starting. Parsing configuration..." << std::endl;
	auto config = parseConfig();

	TLS::Server server(DEFAULT_PORT);
	std::cout << "Listening on port " << DEFAULT_PORT << std::endl;

	server.setup(config.server.cert.c_str(), config.server.key.c_str());
	std::cout << "Configured TLS server" << std::endl;

	std::cout << "Ready to go" << std::endl;

	// Handle requests
	char buffer[BUFFER_SIZE];
	while(true) {
		// Get path
		auto conn = server.acc();
		auto line = conn.recvl();
		if(!line.size()) {
			conn.cl(); continue;
		}

		// Connect to backend
		int backend = socket(AF_INET, SOCK_STREAM, 0);
		if(backend < 0) {
			conn.send("43 zodiac: crowded\r\n");
			conn.cl(); continue;
		}
		if(connect(backend, (struct sockaddr*)&(config.server.saddr),
			       sizeof(sockaddr)) < 0) {
			conn.send("43 zodiac: could not connect to backend\r\n");
			conn.cl(); continue;
		}

		// Send line and client's IP
		send(backend, line.c_str(), line.size(), 0);
		send(backend, "\r\n", 2, 0);
		send(backend, conn.ip.c_str(), conn.ip.size(), 0);
		send(backend, "\r\n", 2, 0);

		// Get response
		while(true) {
			int got = recv(backend, buffer, BUFFER_SIZE, 0);
			if(got <= 0)
				break;
			conn.send(buffer, got);
		}

		close(backend);
		conn.cl();
	}
}
