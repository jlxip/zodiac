#include "tasks.hpp"
#include <iostream>

bool Tasks::proxy(Task* task) {
	// We don't touch task->timeout, it's set by backOut
	// And now we send them to the client in cproxy
	// TODO this is slightly wrong
	task->frontback = FRONTEND;
	return true;
}

int Tasks::cproxy(Task* task) {
	int r = task->conn.send(task->buffer, task->ctr);
	if(!r) {
		// Buffer is full, waiting
		return Tasks::RET_WRITE;
	} else if(r < 0) {
		// Failed :(
		return Tasks::RET_ERROR;
	}

	// Nice. Go back to backOut, since more bytes might be coming
	task->type = Task::BACK_OUT - 1;
	task->type--; // This will get incremented
	return Tasks::RET_OK;
}
