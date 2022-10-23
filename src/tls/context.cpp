#include "tls.hpp"
#include <iostream>
#include <openssl/err.h>

SSL_CTX* TLS::createContext(const char* cert, const char* key) {
	const SSL_METHOD* method = TLS_server_method();
	SSL_CTX* ret = SSL_CTX_new(method);
	if(!ret) {
		std::cerr << "Unable to create SSL context" << std::endl;
		exit(EXIT_FAILURE);
	}

	if(SSL_CTX_use_certificate_file(ret, cert, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}
	if(SSL_CTX_use_PrivateKey_file(ret, key, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}

	return ret;
}
