#include <iostream>
#include <common.hpp>
#include <unistd.h>
#include <sys/mman.h>

Config globalConfig;
TLS::Server globalServer;

int main() {
	std::cout << "Zodiac starting. Parsing configuration..." << std::endl;
	globalConfig = parseConfig();
	globalServer = {0};

	std::cout << "Setting up capsules..." << std::endl;
	for(auto const& x : globalConfig.capsules) {
		std::cout << "-> Enabling " << x.first << std::endl;
		auto* ctx = TLS::createContext
			(x.second.cert.c_str(), x.second.key.c_str());
		globalServer.ctxs[x.first] = ctx;
		globalServer.names[x.second.name] = x.first;
	}
	globalServer.defaultContext = globalServer.ctxs[globalConfig.def];

	std::cout
		<< "Listening on "
		<< globalConfig.listenIP << ':'
		<< globalConfig.listenPort << std::endl;

	// Get a page for cached response movement
	size_t pageSize = sysconf(_SC_PAGE_SIZE);
	char* buffer = (char*)mmap(0, pageSize, PROT_READ | PROT_WRITE,
							   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	// Handle requests
	while(true) {
		auto conn = globalServer.acc();

		// Get server name
		std::string capsuleName;
		bool doomed = false;
		auto it = globalServer.names.find(conn.getSN());
		if(it == globalServer.names.end()) {
			// Unknown. Is default explicitly set?
			if(globalConfig.hasExplicitDefault) {
				// Yep, no worries
				capsuleName = globalConfig.def;
			} else {
				// No; reject the request
				// Can't do it right now, request has to come first,
				//   the client doesn't like it any other way
				doomed = true;
			}
		} else {
			// Nice
			capsuleName = (*it).second;
		}

		// Get frontend timeout
		if(!doomed) {
			size_t ft = globalConfig.capsules[capsuleName].frontTimeout;
			if(!ft)
				ft = globalConfig.frontTimeout;
			conn.setTimeout(ft);
		}

		// Get path. It must be done before sending anything
		auto line = conn.recvl();
		if(!line.size()) {
			conn.cl(); continue;
		}

		// Are we doomed?
		if(doomed) {
			conn.send("51 zodiac: unrecognized server name\r\n");
			conn.cl(); continue;
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
		size_t bt = capsule.backTimeout;
		if(!bt)
			bt = globalConfig.backTimeout;
		timeval timeout;
		timeout.tv_sec = bt;
		timeout.tv_usec = 0;
		setsockopt(backend, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

		// Get response
		while(true) {
			int got = recv(backend, buffer, pageSize, 0);
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
