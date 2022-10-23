#include <iostream>
#include <common.hpp>

int TLS::sniCallback(SSL* ssl, int* al, void* arg) {
	(void)al; (void)arg;
	if(!ssl)
		return SSL_TLSEXT_ERR_NOACK;

	// Get server name
	const char* sn = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
	if(!sn) {
		// No SNI, use default (already set)
		return SSL_TLSEXT_ERR_OK;
	}

	// Got a server name, do I know it?
	auto it = globalServer.names.find(sn);
	if(it != globalServer.names.end()) {
		// I do!
		auto capsule = (*it).second;
		SSL_set_SSL_CTX(ssl, globalServer.ctxs[capsule]);
	}

	return SSL_TLSEXT_ERR_OK;
}
