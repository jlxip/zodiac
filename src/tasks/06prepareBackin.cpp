#include <common.hpp>
#include "tasks.hpp"
#include <cstring>

int task06prepareBackin(SSockets_ctx* ctx) {
	// We want to send the path and the client's IP to the backend
	Data* data = (Data*)(ctx->data);

	// Let's build up a buffer; path is already in it
	auto ip = data->conn.getIP();
	memcpy(data->buffer + data->ctr, ip.c_str(), ip.size());
	data->ctr += ip.size();
	data->buffer[data->ctr++] = '\r';
	data->buffer[data->ctr++] = '\n';

	ctx->disarm = 1; // No timeout here
	ctx->state = STATE_BACKIN;
	return SSockets_RET_OK;
}
