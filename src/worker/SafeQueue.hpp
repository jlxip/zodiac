#ifndef SAFEQUEUE_HPP
#define SAFEQUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T> class SafeQueue {
private:
	std::queue<T> q;
	mutable std::mutex m;
	std::condition_variable c;

public:
	inline void push(const T& x) {
		std::lock_guard<std::mutex> lock(m);
		q.push(x);
		c.notify_one();
	}

	inline T pop() {
		std::unique_lock<std::mutex> lock(m);
		while(q.empty())
			c.wait(lock);
		T ret = q.front();
		q.pop();
		return ret;
	}
};

#endif
