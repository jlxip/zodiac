#include "tls.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/err.h>

TLS::Server::Server(uint16_t port) {
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0) {
		std::cerr << "Unable to create socket" << std::endl;
		exit(EXIT_FAILURE);
	}

	if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		std::cerr << "Unable to bind to port " << port << std::endl;
		exit(EXIT_FAILURE);
	}

	if(listen(sock, 1) < 0) {
		std::cerr << "Unable to listen on port " << port << std::endl;
		exit(EXIT_FAILURE);
	}
}

void TLS::Server::setup(const char* cert, const char* key) {
	const SSL_METHOD* method = TLS_server_method();
	ctx = SSL_CTX_new(method);
	if(!ctx) {
		std::cerr << "Unable to create SSL context" << std::endl;
		exit(EXIT_FAILURE);
	}

	if(SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}
	if(SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}
}

TLS::Connection TLS::Server::acc() {
	sockaddr_in addr;
	uint32_t len = sizeof(addr);

	int client = accept(sock, (struct sockaddr*)&addr, &len);
	if(client < 0) {
		std::cerr << "Unable to accept" << std::endl;
		exit(EXIT_FAILURE);
	}

	SSL* ssl = SSL_new(ctx);
	SSL_set_fd(ssl, client);
	if(SSL_accept(ssl) <= 0) {
		ERR_print_errors_fp(stderr);
		SSL_shutdown(ssl);
		SSL_free(ssl);
		close(client);
		return {};
	}

	// Get client's IP
	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);

	return {ssl, client, ip};
}

void TLS::Server::cl() {
	close(sock);
	SSL_CTX_free(ctx);
}
