#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "upq/updatable_priority_queue.h"
#include <mutex>

/*
	This is an thread-safe min-first updatable priority queue, based on
	the work of @Ten0, which is available at the upq/ submodule.
*/

class Scheduler {
private:
	better_priority_queue::updatable_priority_queue<size_t, int> data;
	std::mutex lock;

	// These handle max-first to min-first conversion
	inline void _set(size_t key, size_t val) { data.set(key, -(int)val); }
	inline size_t _get(size_t key) { return -data.get_priority(key).second; }
	inline size_t _top() { return data.top().key; }

public:
	inline void init(size_t key) {
		std::lock_guard<std::mutex> l(lock);
		data.set(key, 0);
	}

	inline size_t top() {
		std::lock_guard<std::mutex> l(lock);
		return _top();
	}

	inline void inc(size_t key) {
		std::lock_guard<std::mutex> l(lock);
		_set(key, _get(key)+1);
	}

	inline void dec(size_t key) {
		std::lock_guard<std::mutex> l(lock);
		_set(key, _get(key)-1);
	}
};

#endif
