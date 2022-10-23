#ifndef CONN_HPP
#define CONN_HPP

#include <string>
#include <openssl/ssl.h>
#include <unordered_map>

namespace TLS {
	// One of these per connection
	class Connection {
	private:
		SSL* ssl;
		int client;

	public:
		std::string ip;

		inline Connection(SSL* ssl=nullptr, int client=0, std::string ip="")
			: ssl(ssl), client(client), ip(ip)
		{}

		inline std::string getSN() {
			return SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
		}

		std::string recvl(); // Receive line
		bool send(const char* buffer, size_t n);
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
	class Server {
	private:
		int sock;
		timeval timeout;

	public:
		SSL_CTX* defaultContext = nullptr;
		std::unordered_map<std::string, SSL_CTX*> ctxs;
		std::unordered_map<std::string, std::string> names;

		Server() : sock(-1) {}
		Server(uint16_t port);
		inline void setTimeout(size_t s) {
			timeout.tv_sec = s;
			timeout.tv_usec = 0;
		}
		Connection acc();
		void cl();
	};
};

#endif
