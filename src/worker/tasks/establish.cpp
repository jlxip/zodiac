#include "tasks.hpp"
#include <iostream>
#include <ctime>

void Tasks::establish(Task& task) {
	task.conn.createSSL();
	task.timeout = globalConfig.hsTimeout;
	task.deadline = 0;
}

bool Tasks::cestablish(Task& task) {
	if(!task.deadline)
		task.deadline = std::time(nullptr) + task.timeout;

	if(!task.conn.checkHandshake()) {
		bool fail = !task.conn.doHandshake();
		fail = fail || (std::time(nullptr) >= task.deadline);
		if(fail)
			task.type = Task::N_TASKS;
		return false;
	}

	return true;
}
