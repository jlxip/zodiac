#include "tasks.hpp"
#include <iostream>
#include <cstring>
#include <errno.h>
#include <ctime>

void Tasks::backOut(Task& task) {
	// This task gets the output from the backend, and stores it in the buffer
	task.ctr = 0;

	if(!task.deadline) {
		task.timeout = globalConfig.backTimeout;
		task.deadline = std::time(nullptr) + task.timeout;
	}
}

bool Tasks::cbackOut(Task& task) {
	// Let's try to read
	int r = recv(task.backend, task.buffer, task.bufferSize, 0);
	if(r < 0) {
		switch(errno) {
		case EAGAIN:
			if(std::time(nullptr) >= task.deadline) {
				task.conn.send("43 zodiac: response from backend timed out\r\n");
				task.type = Task::N_TASKS;
			}
			return false;
		default:
			task.type = Task::N_TASKS;
			return false;
		}
	} else {
		task.ctr = r;
		task.fullctr += r;

		if(!task.fullctr)
			task.conn.send("43 zodiac: empty response from backend\r\n");

		return true;
	}
}
