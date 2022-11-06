#include <common.hpp>
#include "tasks.hpp"

int task08prepareBackout(SSockets_ctx* ctx) {
	// This task gets the output from the backend, and stores it in the buffer
	Data* data = (Data*)(ctx->data);
	data->ctr = 0;

	if(data->noTimeout) {
		ctx->timeout = globalConfig.backTimeout;
		data->noTimeout = false;
	}

	ctx->fd = data->backend;
	ctx->state = STATE_BACKOUT;
	return SSockets_RET_OK;
}
