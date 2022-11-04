#include <common.hpp>
#include <iostream>
#include <unistd.h>
#include <openssl/err.h>

#define MAX_CONNECTIONS 16

TLS::Server::Server(int) {
	auto port = globalConfig.listenPort;

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(globalConfig.listenIP.c_str());

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0) {
		std::cerr << "Unable to create socket" << std::endl;
		exit(EXIT_FAILURE);
	}

	const int enable = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		std::cerr << "Could not set SO_REUSEADDR" << std::endl;
		exit(EXIT_FAILURE);
	}

	if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		std::cerr << "Unable to bind to port " << port << std::endl;
		exit(EXIT_FAILURE);
	}

	if(!setNonBlocking(sock)) {
		std::cerr << "Could not set non-blocking" << std::endl;
		exit(EXIT_FAILURE);
	}

	if(listen(sock, MAX_CONNECTIONS) < 0) {
		std::cerr << "Unable to listen on port " << port << std::endl;
		exit(EXIT_FAILURE);
	}
}

void TLS::Server::cl() {
	close(sock);
	for(auto& x : ctxs)
		SSL_CTX_free(x.second);
	ctxs.clear();
}
