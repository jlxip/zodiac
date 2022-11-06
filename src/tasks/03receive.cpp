#include <common.hpp>
#include "tasks.hpp"

// Max request length is 1024 according to Gemini specification
#define MAX_REQ_LENGTH 1024

int task03receive(SSockets_ctx* ctx) {
	Data* data = (Data*)(ctx->data);
	int r = data->conn.recv(data->buffer, data->ctr, MAX_REQ_LENGTH);
	if(r <= 0)
		return r; // Something failed along the way

	data->ctr += r;
	char lastChar = data->buffer[data->ctr-1];
	// Read until LF (assumed CR comes before, but it doesn't really matter)
	if(lastChar == '\n') {
		ctx->state = STATE_PREPARE_CONNECT;
		return SSockets_RET_OK;
	}

	// Not ready yet
	if(data->ctr >= MAX_REQ_LENGTH)
		return SSockets_RET_ERROR; // Too big

	return SSockets_RET_OK; // Go again
}
