#include <common.hpp>
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>

void TLS::Connection::createSSL() {
	ssl = SSL_new(globalServer.defaultContext);
	SSL_set_fd(ssl, client);
	SSL_CTX_set_tlsext_servername_callback(globalServer.defaultContext, sniCallback);
	SSL_set_accept_state(ssl); // Not actually needed but good practice
}

int TLS::Connection::doHandshake() {
	if(SSL_is_init_finished(ssl))
		return SSockets_RET_OK;

	int r = SSL_accept(ssl);
	if(r == -1) {
		r = SSL_get_error(ssl, r);
		if(r == SSL_ERROR_WANT_READ)
			return SSockets_RET_READ;
		else if(r == SSL_ERROR_WANT_WRITE)
			return SSockets_RET_WRITE;
		else
			return SSockets_RET_ERROR;
	}

	return SSockets_RET_OK;
}

bool TLS::Connection::checkHandshake() {
	return SSL_is_init_finished(ssl);
}



int TLS::Connection::recv(char* buffer, size_t offset, size_t max) {
	if(!ssl)
		return SSockets_RET_ERROR;

	int r = SSL_read(ssl, buffer+offset, max - offset);
	if(r <= 0) {
		r = SSL_get_error(ssl, r);
		if(r == SSL_ERROR_WANT_READ)
			return SSockets_RET_READ;
		else if(r == SSL_ERROR_WANT_WRITE)
			return SSockets_RET_WRITE;
		return SSockets_RET_ERROR;
	}

	return r; // Got something
}

int TLS::Connection::send(const char* buffer, size_t n) {
	if(!ssl)
		return -1;

	int r = SSL_write(ssl, buffer, n);
	if(r <= 0) {
		r = SSL_get_error(ssl, r);
		if(r == SSL_ERROR_WANT_READ)
			return SSockets_RET_READ;
		else if(r == SSL_ERROR_WANT_WRITE)
			return SSockets_RET_WRITE;
		return SSockets_RET_ERROR;
	}

	return r;
}

void TLS::Connection::close() {
	if(!ssl)
		return;

	SSL_shutdown(ssl);
	SSL_free(ssl);
	::close(client);
}
