#include "worker.hpp"
#include <unistd.h>
#include <sys/mman.h>
#include <iostream>
#include "tasks/tasks.hpp"
#include <stack>
#include <cstring>

std::vector<SafeQueue<Task>> tasks;

[[noreturn]] void* worker(void* _) {
	// ✨ Welcome to the worker procedure ✨
	size_t id = (size_t)_;

	// Let's allocate some buffers, LIFO style, in order
	//   to maximize L1 cache hits in this thread
	size_t nbuffers = globalConfig.buffers / globalConfig.workers;
	size_t pageSize = sysconf(_SC_PAGE_SIZE);
	std::stack<char*> buffers;
	for(size_t i=0; i<nbuffers; ++i)
		buffers.push((char*)mmap(0, pageSize, PROT_READ | PROT_WRITE,
								 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));

	SafeQueue<Task>& mytasks = tasks[id];

	while(true) {
		// Grab task
		Task t(std::move(mytasks.pop()));
		// Does it have a buffer already?
		if(!t.buffer) {
			t.bufferSize = pageSize;
			t.buffer = buffers.top();
			buffers.pop();
		}

		if(t.type == Task::N_TASKS) {
			// One of the tasks marked the whole job as complete
			t.conn.cl(); // Close connection
			sched.dec(id); // Decrease the job count
			memset(t.buffer, 0, pageSize); // Clear the buffer
			buffers.push(t.buffer); // And recycle it
			continue;
		}

		if(!t.waiting) {
			// Not waiting, so a regular task is started
			Tasks::funcs[t.type](t);
			t.waiting = true;
		} else {
			// Waiting. Is it finished?
			if(Tasks::checks[t.type](t)) {
				// Yes, onto the next thing
				t.waiting = false;
				++t.type;
			}
		}

		// Re-insert the (possibly modified) task at the end of the queue
		mytasks.push(t);
	}
}
