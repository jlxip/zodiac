#include "tasks.hpp"
#include <iostream>
#include <cstring>

bool Tasks::backIn(Task* task) {
	// This task sends the path and the client's IP to the backend

	// Let's build up a buffer; path is already in the buffer
	auto ip = task->conn.getIP();
	memcpy(task->buffer+task->ctr, ip.c_str(), ip.size());
	task->ctr += ip.size();
	task->buffer[task->ctr++] = '\r';
	task->buffer[task->ctr++] = '\n';
	task->timeout = 0; // No timeout here
	task->frontback = BACKEND;
	return true;
}

int Tasks::cbackIn(Task* task) {
	int r = send(task->backend, task->buffer, task->ctr, 0);
	if(r >= 0)
		return Tasks::RET_OK;

	switch(errno) {
	case EAGAIN:
		return Tasks::RET_WRITE;
	default:
		return Tasks::RET_ERROR;
	}
}
