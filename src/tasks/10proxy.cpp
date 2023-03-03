#include <common.hpp>
#include "tasks.hpp"

int task10proxy(SSockets_ctx* ctx) {
	// Now we send them to the client in cproxy
	Data* data = (Data*)(ctx->data);
	ctx->fd = data->frontend;

	int r = data->conn.send(data->buffer, data->ctr);
	if(!r || (r < 0 && errno == EAGAIN))
		return SSockets_RET_WRITE; // Buffer is full, waiting
	else if(r < 0)
		return SSockets_RET_ERROR; // Failed :(

	// Nice. Go back to backOut, since more bytes might be coming
	ctx->state = STATE_PREPARE_BACKOUT;
	data->ctr = 0;
	return SSockets_RET_OK;
}
