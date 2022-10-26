#include <iostream>
#include <common.hpp>
#include <unistd.h>
#include <pthread.h>
#include <worker/worker.hpp>
#include <fcntl.h>

const char* defaultConfigPath = "zodiac.conf";

Config globalConfig;
TLS::Server globalServer;
Scheduler sched;

// A gentle introduction to non-blocking sockets:
// https://www.scottklement.com/rpg/socktut/nonblocking.html

int main() {
	std::cout << "Zodiac starting. Parsing configuration..." << std::endl;
	const char* configPath = getenv("ZODIAC_CONFIG");
	if(!configPath)
		configPath = defaultConfigPath;
	if(access(configPath, F_OK) != 0) {
		std::cerr << "Could not open file: " << configPath << std::endl;
		exit(EXIT_FAILURE);
	}

	globalConfig = parseConfig(configPath);
	globalServer = {0}; // Reinitialize globalServer now that there's config

	// Prepare TLS contexts to be used in the server name callback
	std::cout << "Setting up capsules..." << std::endl;
	for(auto const& x : globalConfig.capsules) {
		std::cout << "-> Enabling " << x.first << std::endl;
		auto* ctx = TLS::createContext
			(x.second.cert.c_str(), x.second.key.c_str());
		globalServer.ctxs[x.first] = ctx;
		globalServer.names[x.second.name] = x.first;
	}
	globalServer.defaultContext = globalServer.ctxs[globalConfig.def];

	// Spawn worker threads
	size_t workers = globalConfig.workers;
	if(!workers)
		globalConfig.workers = workers = nproc(); // Overwriting config
	std::cout << "Spawning " << workers << " worker threads..." << std::endl;
	tasks = std::move(std::vector<SafeQueue<Task>>(workers));
	for(size_t i=0; i<workers; ++i) {
		sched.init(i);
		pthread_t thread;
		pthread_create(&thread, NULL, worker, (void*)i);
	}

	std::cout
		<< "Listening on "
		<< globalConfig.listenIP << ':'
		<< globalConfig.listenPort << std::endl;

	// Handle requests
	while(true) {
		// Accept a connection
		sockaddr_in addr;
		uint32_t len = sizeof(addr);
		int client = accept(globalServer.sock, (struct sockaddr*)&addr, &len);
		if(client < 0)
			continue;

		// We're going non-blocking
		int flags = fcntl(client, F_GETFL);
		if(flags < 0) {
			close(client);
			continue;
		}
		if(fcntl(client, F_SETFL, flags | O_NONBLOCK) < 0) {
			close(client);
			continue;
		}

		// Create the task
		Task task;
		task.conn = {client, addr};

		// Assign it to a worker thread
		size_t id = sched.top();
		tasks[id].push(task);
		sched.inc(id);

		/*
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
		conn.cl();*/
	}
}
