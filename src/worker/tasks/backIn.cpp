#include "tasks.hpp"
#include <iostream>
#include <cstring>

void Tasks::backIn(Task& task) {
	// This task sends the path and the client's IP to the backend

	// Let's build up a buffer; path is already in the buffer
	auto ip = task.conn.getIP();
	memcpy(task.buffer+task.ctr, ip.c_str(), ip.size());
	task.ctr += ip.size();
	task.buffer[task.ctr++] = '\r';
	task.buffer[task.ctr++] = '\n';
}

bool Tasks::cbackIn(Task& task) {
	int r = send(task.backend, task.buffer, task.ctr, 0);
	if(r >= 0) {
		task.deadline = 0; // Set 0 deadline for backOut
		return true;
	}

	switch(errno) {
	case EAGAIN:
		return false;
	default:
		task.type = Task::N_TASKS;
		return false;
	}
}
