#ifndef CONN_HPP
#define CONN_HPP

#include <string>
#include <openssl/ssl.h>

namespace TLS {
	class Connection {
	private:
		SSL* ssl;
		int client;

	public:
		std::string ip;

		inline Connection(SSL* ssl=nullptr, int client=0, std::string ip="")
			: ssl(ssl), client(client), ip(ip)
		{}

		std::string recvl(); // Receive line
		bool send(const char* buffer, size_t n);
		inline bool send(const std::string& msg) {
			return send(&msg[0], msg.size());
		}

		void cl();
	};

	class Server {
	private:
		int sock;
		SSL_CTX* ctx;
		timeval timeout;

	public:
		Server(uint16_t port);
		void setup(const char* cert, const char* key); // Paths
		inline void setTimeout(size_t s) {
			timeout.tv_sec = s;
			timeout.tv_usec = 0;
		}
		Connection acc();
		void cl();
	};
};

#endif
