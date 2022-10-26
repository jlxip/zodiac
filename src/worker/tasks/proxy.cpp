#include "tasks.hpp"
#include <iostream>

void Tasks::proxy(Task& task) {
	// backOut received some bytes, allegedly. Was it empty?
	if(!task.ctr) {
		task.type = Task::N_TASKS;
		return;
	}

	// And now we send them to the client in cproxy
}

bool Tasks::cproxy(Task& task) {
	int r = task.conn.send(task.buffer, task.ctr);
	if(!r) {
		// Buffer full, waiting
		return false;
	} else if(r < 0) {
		// Failed :(
		task.type = Task::N_TASKS;
		return false;
	}

	// Nice. Go back to backOut, since more bytes might be coming
	task.type = Task::BACK_OUT;
	task.waiting = false;
	return false;
}
