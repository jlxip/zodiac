#include <common.hpp>
#include "tasks.hpp"

int task07backin(SSockets_ctx* ctx) {
	Data* data = (Data*)(ctx->data);
	int r = send(data->backend, data->buffer, data->ctr, 0);
	if(r >= 0) {
		ctx->state = STATE_PREPARE_BACKOUT;
		return SSockets_RET_OK;
	}

	if(errno == EAGAIN)
		return SSockets_RET_WRITE;
	else
		return SSockets_RET_ERROR;
}
