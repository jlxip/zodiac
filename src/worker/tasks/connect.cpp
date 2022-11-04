#include "tasks.hpp"
#include <iostream>
#include <fcntl.h>
#include <errno.h>

bool Tasks::connect(Task* task) {
	// Are we doomed?
	if(task->doomed) {
		task->conn.send("51 zodiac: unrecognized server name\r\n");
		return false;
	}

	// Does the request end in CRLF?
	if(task->ctr < 2) {
		task->conn.send("59 zodiac: no CRLF\r\n");
		return false;
	} else if(task->buffer[task->ctr-2] != '\r' || task->buffer[task->ctr-1] != '\n') {
		task->conn.send("59 zodiac: no CRLF\r\n");
		return false;
	}

	// Create socket to backend
	task->backend = socket(AF_INET, SOCK_STREAM, 0);
	if(task->backend < 0) {
		task->conn.send("43 zodiac: crowded\r\n");
		return false;
	}

	// Non-blocking
	if(!setNonBlocking(task->backend)) {
		task->conn.send("41 zodiac: error non-blocking\r\n");
		return false;
	}

	// Set timeout
	task->timeout = task->capsule->backTimeout;
	if(!task->timeout)
		task->timeout = globalConfig.backTimeout;

	task->frontback = BACKEND;
	return true;
}

int Tasks::cconnect(Task* task) {
	// Connect now
	auto r = connect(task->backend, (struct sockaddr*)&(task->capsule->saddr),
					 sizeof(sockaddr));

	if(r == 0)
		return Tasks::RET_OK;

	switch(errno) {
	case EINPROGRESS:
	case EAGAIN:
		return Tasks::RET_WRITE;
	case EISCONN:
		// Got it
		return Tasks::RET_OK;
	default:
		// Failed :/
		task->conn.send("43 zodiac: could not connect to backend\r\n");
		return Tasks::RET_ERROR;
	}
}
