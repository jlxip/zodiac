#include <iostream>
#include <common.hpp>
#include <unistd.h>
#include "tasks/tasks.hpp"
#include <sys/mman.h>

const char* defaultConfigPath = "zodiac.conf";

Config globalConfig;
TLS::Server globalServer;

void hangup(SSockets_ctx* ctx);
void timeout(SSockets_ctx* ctx);
void destroy(SSockets_ctx* ctx);

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

	std::cout
		<< "Listening on "
		<< globalConfig.listenIP << ':'
		<< globalConfig.listenPort << std::endl;

	SSockets_addState(task00init);
	SSockets_addState(task01handshake);
	SSockets_addState(task02identify);
	SSockets_addState(task03receive);
	SSockets_addState(task04prepareConnect);
	SSockets_addState(task05connect);
	SSockets_addState(task06prepareBackin);
	SSockets_addState(task07backin);
	SSockets_addState(task08prepareBackout);
	SSockets_addState(task09backout);
	SSockets_addState(task10proxy);

	SSockets_setHangupCallback(hangup);
	SSockets_setTimeoutCallback(timeout);
	SSockets_setDestroyCallback(destroy);

	SSockets_run(globalConfig.listenIP.c_str(),
				 globalConfig.listenPort,
				 globalConfig.workers);
}

void hangup(SSockets_ctx* ctx) {
	Data* data = (Data*)(ctx->data);
	if(ctx->state == STATE_CONNECT)
		data->conn.send("43 zodiac: could not connect to backend\r\n");
	else if(ctx->state > STATE_CONNECT)
		data->conn.send("43 zodiac: connection closed\r\n");
}

void timeout(SSockets_ctx* ctx) {
	Data* data = (Data*)(ctx->data);
	if(ctx->state == STATE_CONNECT)
		data->conn.send("43 zodiac: connection timed out\r\n");
	else if(ctx->state == STATE_BACKOUT)
		data->conn.send("43 zodiac: response from backend timed out\r\n");
}

#include <sys/epoll.h>
extern int SSockets_epollfd;
void destroy(SSockets_ctx* ctx) {
	Data* data = (Data*)(ctx->data);

	epoll_ctl(SSockets_epollfd, EPOLL_CTL_DEL, data->frontend, nullptr);
	data->conn.close();

	if(data->backend) {
		epoll_ctl(SSockets_epollfd, EPOLL_CTL_DEL, data->backend, nullptr);
		close(data->backend);
	}

	munmap(data->buffer, data->bufferSize);
	delete data;
}
