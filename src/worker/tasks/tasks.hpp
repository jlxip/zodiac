#ifndef TASKS_HPP
#define TASKS_HPP

#include <worker/worker.hpp>

/*
	These are the steps (tasks) that form a full job (request processing)
	Two types: CPU-intensive and IO-intensive.
	CPU-intensive are performed once.
	IO-intensive might be performed multiple times until all data is read
	  or written.
*/

namespace Tasks {
	const int RET_OK = 0;
	const int RET_READ = -1;
	const int RET_WRITE = -2;
	const int RET_ERROR = -3;

	bool establish(Task*); int cestablish(Task*);
	bool receive(Task*); int creceive(Task*);
	bool connect(Task*); int cconnect(Task*);
	bool backIn(Task*); int cbackIn(Task*);
	bool backOut(Task*); int cbackOut(Task*);
	bool proxy(Task*); int cproxy(Task*);

	typedef bool (*cpuwork)(Task*);
	typedef int (*iowork)(Task*);

	const cpuwork cpuworks[Task::N_TASKS] = {establish, receive, connect,
											 backIn, backOut, proxy};
	const iowork ioworks[Task::N_TASKS] = {cestablish, creceive, cconnect,
										   cbackIn, cbackOut, cproxy};
};

#endif
