#include "tasks.hpp"
#include <iostream>

bool Tasks::establish(Task* task) {
	task->conn.createSSL();
	task->timeout = globalConfig.hsTimeout;
	task->frontback = FRONTEND;
	return true;
}

int Tasks::cestablish(Task* task) {
	if(!task->conn.checkHandshake())
		return task->conn.doHandshake();
	return Tasks::RET_OK;
}
