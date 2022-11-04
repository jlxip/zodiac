#include "tasks.hpp"
#include <iostream>
#include <cstring>
#include <errno.h>

bool Tasks::backOut(Task* task) {
	// This task gets the output from the backend, and stores it in the buffer
	task->ctr = 0;

	if(!task->timeout)
		task->timeout = globalConfig.backTimeout;

	task->frontback = BACKEND;
	return true;
}

int Tasks::cbackOut(Task* task) {
	// Let's try to read
	int r = recv(task->backend, task->buffer, task->bufferSize, 0);
	if(r < 0) {
		switch(errno) {
		case EAGAIN:
			return Tasks::RET_READ;
		default:
			return Tasks::RET_ERROR;
		}
	} else {
		task->ctr = r;
		task->fullctr += r;

		if(!task->fullctr)
			task->conn.send("43 zodiac: empty response from backend\r\n");
		if(!task->ctr)
			return Tasks::RET_ERROR;

		return Tasks::RET_OK;
	}
}
