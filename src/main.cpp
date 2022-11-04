#include <iostream>
#include <common.hpp>
#include <unistd.h>
#include <pthread.h>
#include <worker/worker.hpp>
#include <signal.h>
#include <sys/epoll.h>

const char* defaultConfigPath = "zodiac.conf";

Config globalConfig;
TLS::Server globalServer;

// A gentle introduction to non-blocking sockets:
// https://www.scottklement.com/rpg/socktut/nonblocking.html

// The advices from the following post are followed:
// https://idea.popcount.org/2017-02-20-epoll-is-fundamentally-broken-12/

const size_t listenEvents = EPOLLIN | EPOLLEXCLUSIVE;

int epoll_fd;

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

	// Ignore SIGPIPE, the standard practice in all TCP servers
	signal(SIGPIPE, SIG_IGN);

	// Build epoll fd
	if((epoll_fd = epoll_create1(0)) < 0) {
		std::cerr << "Could not create epoll fd" << std::endl;
		exit(1);
	}

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
	for(size_t i=1; i<workers; ++i) {
		pthread_t thread;
		pthread_create(&thread, NULL, worker, (void*)i);
	}

	std::cout
		<< "Listening on "
		<< globalConfig.listenIP << ':'
		<< globalConfig.listenPort << std::endl;

	// Add listening socket to epoll
	Event* evt = new Event;
	evt->type = Event::LISTEN;
	evt->u.listenfd = globalServer.sock;
	epoll_event ev;
	ev.events = listenEvents;
	ev.data.ptr = evt;
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, globalServer.sock, &ev) == -1) {
		std::cerr << "Could not add to epoll" << std::endl;
		exit(1);
	}

	worker(nullptr);
}
