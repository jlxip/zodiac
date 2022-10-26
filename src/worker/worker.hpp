#ifndef WORKER_H
#define WORKER_H

#include <mutex>
#include "SafeQueue.hpp"
#include "scheduler/scheduler.hpp"
#include <common.hpp>
#include <ctime>

/*
	A job is defined as the process of serving a capsule, from beginning to end.
	Jobs are balanced across worker threads, and they don't switch, in order
	  to reduce L1 cache misses.
	Jobs are split into tasks, which are parts of the job that would finish
	  in blocking. These are separated so that each thread can avoid blocking
	  and, instead, use async operations to be able to switch to another task.
*/

struct Task {
	// These are the task types
	static const size_t ESTABLISH = 0; // Ends performing the TLS handshake
	static const size_t RECEIVE = 1;   // Ends receiving the path
	static const size_t CONNECT = 2;   // Ends connecting to the backend
	static const size_t BACK_IN = 3;   // Ends sending the request to the backend
	static const size_t BACK_OUT = 4;  // Ends receiving the response from the backend
	static const size_t PROXY = 5;     // Redirects from backend to client
	static const size_t N_TASKS = 6;

	size_t type = ESTABLISH;
	bool waiting = false;

	// State
	TLS::Connection conn;
	bool doomed = false;
	CapsuleConfig* capsule = nullptr;
	int backend = 0;

	char* buffer = nullptr;
	size_t bufferSize = 0;
	size_t ctr = 0; // Bytes written in buffer

	// These get recycled for side
	std::time_t timeout = 0, deadline = 0;
};

extern std::vector<SafeQueue<Task>> tasks;

// How many jobs (not tasks) assigned to each worker thread
extern Scheduler sched;

[[noreturn]] void* worker(void* _);
void goodjob(size_t id);

#endif
