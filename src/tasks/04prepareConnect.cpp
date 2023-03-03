#include <common.hpp>
#include "tasks.hpp"
#include <fcntl.h>

extern "C" int setNonBlocking(int fd); // Stealing from SSockets

int task04prepareConnect(SSockets_ctx* ctx) {
	Data* data = (Data*)(ctx->data);

	// Are we doomed?
	if(data->doomed) {
		data->conn.send("51 zodiac: unrecognized server name\r\n");
		return SSockets_RET_ERROR;
	}

	// Does the request end in CRLF?
	if(data->ctr < 2) {
		data->conn.send("59 zodiac: too short!\r\n");
		return SSockets_RET_ERROR;
	} else if(data->buffer[data->ctr-2] != '\r') {
		// Fix LF to CRLF
		// This is safe since ctr <= MAX_REQ_LENGTH <<< PAGE_SIZE
		data->buffer[data->ctr-1] = '\r';
		data->buffer[data->ctr] = '\n';
		data->buffer[data->ctr+1] = '\0';
	}

	// Create socket to backend
	data->backend = socket(AF_INET, SOCK_STREAM, 0);
	if(data->backend < 0) {
		data->conn.send("43 zodiac: crowded\r\n");
		return SSockets_RET_ERROR;
	}

	// Non-blocking
	if(setNonBlocking(data->backend) != 0) {
		data->conn.send("41 zodiac: error non-blocking\r\n");
		return SSockets_RET_ERROR;
	}

	// Set timeout
	ctx->timeout = data->capsule->backTimeout;
	if(!ctx->timeout)
		ctx->timeout = globalConfig.backTimeout;

	// Now we pick one of the backends, round-robin
	// It's worth checking if there's more than 1 so that mutex is not acquired
	if(data->capsule->addrs.size() > 1) {
		data->capsule->addrLock.lock();
		size_t which = data->capsule->addr++;
		data->capsule->addr %= data->capsule->addrs.size();
		data->capsule->addrLock.unlock();
		data->saddr = data->capsule->addrs[which];
	} else {
		data->saddr = data->capsule->addrs[0];
	}

	ctx->fd = data->backend;
	ctx->state = STATE_CONNECT;
	return SSockets_RET_OK;
}
