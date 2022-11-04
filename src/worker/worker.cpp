#include "worker.hpp"
#include <unistd.h>
#include <sys/mman.h>
#include <iostream>
#include "tasks/tasks.hpp"
#include <stack>
#include <cstring>
#include <sys/epoll.h>
#include <sys/timerfd.h>

#define MAX_EVENTS 32

// Sockets use edge triggered, EPOLLONESHOT
const size_t basicEvents = EPOLLET | EPOLLONESHOT;
const size_t timerEvents = EPOLLET | EPOLLIN;

static size_t pageSize = 0;

[[noreturn]] void* worker(void*) {
	// ✨ Welcome to the worker procedure ✨

	pageSize = sysconf(_SC_PAGE_SIZE);

	epoll_event events[MAX_EVENTS];
	while(true) {
		// Wait for events
		size_t nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		// Process each one
		for(size_t i=0; i<nfds; ++i) {
			Event* evt = (Event*)(events[i].data.ptr);

			auto ev = events[i].events; // Flags
			Task* task = nullptr;

			if(evt->type == Event::LISTEN) {
				// New connection
				struct sockaddr_in addr;
				uint32_t len = sizeof(addr);
				int conn = accept(evt->u.listenfd, (struct sockaddr*)&addr, &len);
				if(conn == -1)
					continue;


				// Set up the task object
				task = new Task;
				task->conn = {conn, addr};
				task->conn.setNonBlocking();
				task->conn.createSSL();
				task->bufferSize = pageSize;
				task->buffer = (char*)mmap(0, pageSize, PROT_READ | PROT_WRITE,
										   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

				// Create its event
				evt = new Event;
				evt->type = Event::TASK;
				evt->u.task = task;

				// And listen to it
				epoll_event ev;
				ev.events = basicEvents;
				ev.data.ptr = evt;
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn, &ev);


				// Set up task's timerfd, for timeouts
				task->timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
				itimerspec spec;
				spec.it_value.tv_sec = spec.it_value.tv_nsec = 0;
				spec.it_interval.tv_sec = spec.it_interval.tv_nsec = 0;
				timerfd_settime(task->timerfd, 0, &spec, nullptr);

				// Create its event
				Event* newevt = new Event;
				newevt->type = Event::TIMEOUT;
				newevt->u.task = task;

				// And listen to it
				ev.events = timerEvents;
				ev.data.ptr = newevt;
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, task->timerfd, &ev);


				// Run the first CPU-intensive task
				if(!Tasks::cpuworks[task->type](task)) {
					closeAndDestroy(task);
					continue;
				} else if(task->timeout) {
					// New timeout
					itimerspec spec;
					spec.it_value.tv_sec = task->timeout;
					spec.it_value.tv_nsec = 0;
					spec.it_interval.tv_sec = spec.it_interval.tv_nsec = 0;
					timerfd_settime(task->timerfd, 0, &spec, nullptr);
				}
				// Fallthrough to run the first IO-intensive task
			} else if(evt->type == Event::TIMEOUT) {
				// Something timed out
				task = evt->u.task;
				if(task->type == 2)
					task->conn.send("43 zodiac: connection timed out\r\n");
				else if(task->type == 4)
					task->conn.send("43 zodiac: response from backend timed out\r\n");
				closeAndDestroy(task);
				delete evt;
				continue;
			} else {
				// Regular IO event
				task = evt->u.task;
			}

			if(ev & EPOLLHUP) {
				if(task->type == 2)
					task->conn.send("43 zodiac: could not connect to backend\r\n");
				else if(task->type > 2)
					task->conn.send("43 zodiac: connection closed\r\n");
				closeAndDestroy(task);
				delete evt;
				continue;
			}

			// If reached this point, the event is IN or OUT

			int result = Tasks::RET_OK;
			while(result == Tasks::RET_OK) {
				epoll_event ev;
				ev.events = basicEvents;
				ev.data.ptr = evt;

				result = Tasks::ioworks[task->type](task);
				switch(result) {
				case Tasks::RET_OK:
					if(task->disarmTimeout) {
						itimerspec spec;
						spec.it_value.tv_sec = spec.it_value.tv_nsec = 0;
						spec.it_interval.tv_sec = spec.it_interval.tv_nsec = 0;
						timerfd_settime(task->timerfd, 0, &spec, nullptr);
						task->disarmTimeout = false;
					}

					task->type++;
					if(!Tasks::cpuworks[task->type](task)) {
						closeAndDestroy(task);
						delete evt;
						result = Tasks::RET_ERROR;
					} else if(task->timeout) {
						// New timeout
						itimerspec spec;
						spec.it_value.tv_sec = task->timeout;
						spec.it_value.tv_nsec = 0;
						spec.it_interval.tv_sec = spec.it_interval.tv_nsec = 0;
						timerfd_settime(task->timerfd, 0, &spec, nullptr);
					}
					break;
				case Tasks::RET_READ:
					ev.events |= EPOLLIN;
					break;
				case Tasks::RET_WRITE:
					ev.events |= EPOLLOUT;
					break;
				case Tasks::RET_ERROR:
					closeAndDestroy(task);
					delete evt;
					break;
				}

				if(result != Tasks::RET_OK && result != Tasks::RET_ERROR) {
					// Waiting for something, so let's add to epoll
					int fd;
					bool added;
					if(task->frontback == FRONTEND) {
						fd = task->conn.getSocket();
						added = true;
					} else {
						fd = task->backend;
						added = task->backendAdded;
						task->backendAdded = true;
					}

					if(!added)
						epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
					else
						epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
				}
			}
		}
	}
}

void closeAndDestroy(Task* task) {
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, task->conn.getSocket(), nullptr);
	task->conn.cl(); // Close connection

	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, task->timerfd, nullptr);
	close(task->timerfd);

	if(task->backend) {
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, task->backend, nullptr);
		close(task->backend);
	}

	munmap(task->buffer, pageSize);
	delete task;
}
