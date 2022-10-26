#include "tasks.hpp"
#include <iostream>

// Max request length is 1024 according to Gemini specification
#define MAX_REQ_LENGTH 1024

void Tasks::receive(Task& task) {
	/*
		TLS handshake has just finished. Now, we wait for the path.
		Let's first get the server name for two reasons:
		  i) "Doom" the connection to fail in case it's invalid and there's
		       no default. It can't fail right now, the response MUST be
		       sent after the request. Clients don't like it any other way.
		  ii) Set the proper timeout for the read.
	*/

	// Get server name (and a pointer to CapsuleConfig if possible)
	auto it = globalServer.names.find(task.conn.getSN());
	if(it == globalServer.names.end()) {
		// Unknown. Is default explicitly set?
		if(globalConfig.hasExplicitDefault) {
			// Yep, no worries
			task.capsule = &(globalConfig.capsules[globalConfig.def]);
		} else {
			// No; doom the connection
			task.doomed = true;
		}
	} else {
		// Nice
		task.capsule = &(globalConfig.capsules[(*it).second]);
	}

	// Set the frontend timeout
	task.timeout = globalConfig.frontTimeout;
	if(!task.doomed) {
		// If it's not doomed, then there's a capsule attached, so let's
		//   set its timeout, in case there is one.
		size_t specific = task.capsule->frontTimeout;
		if(specific)
			task.timeout = specific;
	}

	// Now we get the path at creceive()
	task.deadline = 0;
}

bool Tasks::creceive(Task& task) {
	if(!task.deadline)
		task.deadline = std::time(nullptr) + task.timeout;

	int r = task.conn.recv(task.buffer, task.ctr, MAX_REQ_LENGTH);
	if(!r) {
		// Nothing new
		if(std::time(nullptr) >= task.deadline)
			task.type = Task::N_TASKS; // Timeout!
		return false;
	} else if(r < 0) {
		// Something failed along the way
		task.type = Task::N_TASKS;
		return false;
	}

	task.ctr += r;
	char lastChar = task.buffer[task.ctr-1];
	// Read until LF (assumed CR comes before, but it doesn't really matter)
	if(lastChar == '\n')
		return true; // All set!

	// Not ready yet
	bool bad = std::time(nullptr) >= task.deadline; // Timeout
	bad = bad || (task.ctr >= MAX_REQ_LENGTH); // or too big
	if(bad)
		task.type = Task::N_TASKS;
	return false;
}
