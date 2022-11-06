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
		data->conn.send("59 zodiac: no CRLF\r\n");
		return SSockets_RET_ERROR;
	} else if(data->buffer[data->ctr-2] != '\r' || data->buffer[data->ctr-1] != '\n') {
		data->conn.send("59 zodiac: no CRLF\r\n");
		return SSockets_RET_ERROR;
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

	ctx->fd = data->backend;
	ctx->state = STATE_CONNECT;
	return SSockets_RET_OK;
}
