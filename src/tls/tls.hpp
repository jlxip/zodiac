#ifndef CONN_HPP
#define CONN_HPP

#include <string>
#include <openssl/ssl.h>
#include <unordered_map>
#include <arpa/inet.h>

/*
	This is sort of a simple TLS wrapper, but very specific for the
	  purposes of this project. Non-blocking.
*/

namespace TLS {
	// One of these per connection
	class Connection {
	private:
		SSL* ssl = nullptr;
		int client = 0;
		sockaddr_in addr;

	public:
		Connection() = default;
		Connection(int client, sockaddr_in addr)
			: client(client), addr(addr)
		{}
		inline int getSocket() const { return client; }

		// Before handshake
		bool setNonBlocking();
		void createSSL();
		int doHandshake();
		bool checkHandshake();

		// After handshake
		inline std::string getSN() {
			return SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
		}

		inline std::string getIP() {
			char aux[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(addr.sin_addr), aux, INET_ADDRSTRLEN);
			return aux;
		}

		int recv(char* buffer, size_t offset, size_t max);
		int send(const char* buffer, size_t n);
		inline bool send(const std::string& msg) {
			return send(&msg[0], msg.size());
		}

		void cl();
	};

	// One of these per capsule
	SSL_CTX* createContext(const char* cert, const char* key);
	// For the singleton
	int sniCallback(SSL* ssl, int* al, void* arg);

	// Singleton
	struct Server {
		int sock;
		SSL_CTX* defaultContext = nullptr;
		std::unordered_map<std::string, SSL_CTX*> ctxs;
		std::unordered_map<std::string, std::string> names;

		Server() : sock(-1) {}
		Server(int);
		void cl();
	};
};

#endif
