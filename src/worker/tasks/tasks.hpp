#ifndef TASKS_HPP
#define TASKS_HPP

#include <worker/worker.hpp>

/*
	These are the steps (tasks) that form a full job
	The ones that start with an extra 'c' are checks: they are executed
	  repeatedly until they return true. This is great for non-blocking
	  operations.
	The ones that do not, are only executed once, so they're for the rest
	  of the initialization which would not block in any scenario.
*/

namespace Tasks {
	void establish(Task&); bool cestablish(Task&);
	void receive(Task&); bool creceive(Task&);
	void connect(Task&); bool cconnect(Task&);
	void backIn(Task&); bool cbackIn(Task&);
	void backOut(Task&); bool cbackOut(Task&);
	void proxy(Task&); bool cproxy(Task&);

	typedef void (*func)(Task&);
	typedef bool (*check)(Task&);

	const func funcs[Task::N_TASKS] = {establish, receive, connect,
									   backIn, backOut, proxy};
	const check checks[Task::N_TASKS] = {cestablish, creceive, cconnect,
										 cbackIn, cbackOut, cproxy};
};

#endif
