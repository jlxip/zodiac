#include "tasks.hpp"
#include <iostream>
#include <fcntl.h>
#include <ctime>
#include <errno.h>

void Tasks::connect(Task& task) {
	// Are we doomed?
	if(task.doomed) {
		task.conn.send("51 zodiac: unrecognized server name\r\n");
		task.type = Task::N_TASKS;
		return;
	}

	// Does the request end in CRLF?
	if(task.ctr < 2) {
		task.conn.send("59 zodiac: no CRLF\r\n");
		task.type = Task::N_TASKS;
		return;
	} else if(task.buffer[task.ctr-2] != '\r' || task.buffer[task.ctr-1] != '\n') {
		task.conn.send("59 zodiac: no CRLF\r\n");
		task.type = Task::N_TASKS;
		return;
	}

	// Create socket to backend
	task.backend = socket(AF_INET, SOCK_STREAM, 0);
	if(task.backend < 0) {
		task.conn.send("43 zodiac: crowded\r\n");
		task.type = Task::N_TASKS;
		return;
	}

	// Non-blocking
	int flags = fcntl(task.backend, F_GETFL);
	if(flags < 0) {
		task.conn.send("41 zodiac: error 1\r\n");
		task.type = Task::N_TASKS;
		return;
	}
	if(fcntl(task.backend, F_SETFL, flags | O_NONBLOCK) < 0) {
		task.conn.send("41 zodiac: error 2\r\n");
		task.type = Task::N_TASKS;
		return;
	}

	// Set timeout
	task.timeout = task.capsule->backTimeout;
	if(!task.timeout)
		task.timeout = globalConfig.backTimeout;
	task.deadline = std::time(nullptr) + task.timeout;
}

bool Tasks::cconnect(Task& task) {
	// Connect now
	connect(task.backend, (struct sockaddr*)&(task.capsule->saddr),
			sizeof(sockaddr));

	switch(errno) {
	case EINPROGRESS:
	case EAGAIN:
		return false;
	case EISCONN:
		// Got it
		return true;
	default:
		// Failed :/
		task.conn.send("43 zodiac: could not connect to backend\r\n");
		task.type = Task::N_TASKS;
		return false;
	}
}
