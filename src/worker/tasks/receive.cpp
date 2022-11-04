#include "tasks.hpp"
#include <iostream>

// Max request length is 1024 according to Gemini specification
#define MAX_REQ_LENGTH 1024

bool Tasks::receive(Task* task) {
	/*
		TLS handshake has just finished. Now, we wait for the path.
		Let's first get the server name for two reasons:
		  i) "Doom" the connection to fail in case it's invalid and there's
		       no default. It can't fail right now, the response MUST be
		       sent after the request. Clients don't like it any other way.
		  ii) Set the proper timeout for the read.
	*/

	// Get server name (and a pointer to CapsuleConfig if possible)
	auto it = globalServer.names.find(task->conn.getSN());
	if(it == globalServer.names.end()) {
		// Unknown. Is default explicitly set?
		if(globalConfig.hasExplicitDefault) {
			// Yep, no worries
			task->capsule = &(globalConfig.capsules[globalConfig.def]);
		} else {
			// No; doom the connection
			task->doomed = true;
		}
	} else {
		// Nice
		task->capsule = &(globalConfig.capsules[(*it).second]);
	}

	// Set the frontend timeout
	task->timeout = globalConfig.frontTimeout;
	if(!task->doomed) {
		// If it's not doomed, then there's a capsule attached, so let's
		//   set its timeout, in case there is one.
		size_t specific = task->capsule->frontTimeout;
		if(specific)
			task->timeout = specific;
	}

	// Now we get the path at creceive()
	task->frontback = FRONTEND;
	return true;
}

int Tasks::creceive(Task* task) {
	int r = task->conn.recv(task->buffer, task->ctr, MAX_REQ_LENGTH);
	if(r == Tasks::RET_ERROR)
		return r; // Something failed along the way

	if(r <= 0)
		return Tasks::RET_READ; // Nothing new

	task->ctr += r;
	char lastChar = task->buffer[task->ctr-1];
	// Read until LF (assumed CR comes before, but it doesn't really matter)
	if(lastChar == '\n')
		return Tasks::RET_OK;
	// Not ready yet

	if(task->ctr >= MAX_REQ_LENGTH)
		return Tasks::RET_ERROR; // Too big

	return Tasks::RET_READ;
}
