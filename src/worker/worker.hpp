#ifndef WORKER_H
#define WORKER_H

#include <mutex>
#include <common.hpp>

#define FRONTEND false
#define BACKEND  true

struct Task {
	// These are the task types
	static const size_t ESTABLISH = 0; // Ends performing the TLS handshake
	static const size_t RECEIVE = 1;   // Ends receiving the path
	static const size_t CONNECT = 2;   // Ends connecting to the backend
	static const size_t BACK_IN = 3;   // Ends sending the request to the backend
	static const size_t BACK_OUT = 4;  // Ends receiving the response from the backend
	static const size_t PROXY = 5;     // Redirects from backend to client
	static const size_t N_TASKS = 6;

	size_t type = ESTABLISH;

	// State
	TLS::Connection conn;
	bool doomed = false;
	CapsuleConfig* capsule = nullptr;
	int backend = 0;
	bool backendAdded = false; // Wether EPOLL_CTL_ADD was called on the backend
	bool frontback = false; // Waiting for FRONTEND or BACKEND

	char* buffer = nullptr;
	size_t bufferSize = 0;
	size_t ctr = 0; // Bytes written in buffer

	// All bytes written in buffer in the past from backOut
	// This is just to return an error if response was empty
	size_t fullctr = 0;

	// Timeouts
	int timeout = 0;
	int timerfd = 0;
	bool disarmTimeout = false;
};

struct Event {
	static const size_t LISTEN = 0;
	static const size_t TASK = 1;
	static const size_t TIMEOUT = 2;

	size_t type;

	union {
		int listenfd;
		Task* task;
	} u;
};

[[noreturn]] void* worker(void*);
void closeAndDestroy(Task* task);

#endif
