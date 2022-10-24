#include "tls.hpp"
#include <unistd.h>
#include <sys/socket.h>

void TLS::Connection::setTimeout(size_t s) {
	timeval to;
	to.tv_sec = s;
	to.tv_usec = 0;
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof to);
}

std::string TLS::Connection::recvl() {
	if(!ssl)
		return {};

	std::string ret;

	// Just read until CR (assumed LF comes after), or LF
	// TODO: can this be an issue with unicode URLs?
	char c;
	while(true) {
		if(SSL_read(ssl, &c, 1) <= 0)
			return {};
		if(c == '\r' || c == '\n')
			break;
		ret += c;
	}

	return ret;
}

bool TLS::Connection::send(const char* buffer, size_t n) {
	if(!ssl)
		return false;
	return SSL_write(ssl, buffer, n) > 0;
}

void TLS::Connection::cl() {
	if(!ssl)
		return;

	SSL_shutdown(ssl);
	SSL_free(ssl);
	close(client);
}
