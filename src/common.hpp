#ifndef COMMON_HPP
#define COMMON_HPP

#include <tls/tls.hpp>
#include <config/config.hpp>
#include <vector>

// After initialization, these two turn read-only
extern Config globalConfig;
extern TLS::Server globalServer;

extern int epoll_fd;

// aux.cpp
size_t nproc();
std::vector<std::string> commaSplit(const char* str);
bool setNonBlocking(int fd);

#endif
