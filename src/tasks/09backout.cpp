#include <common.hpp>
#include "tasks.hpp"

int task09backout(SSockets_ctx* ctx) {
	// Let's try to read
	Data* data = (Data*)(ctx->data);
	int r = recv(data->backend, data->buffer, data->bufferSize, 0);
	if(r < 0) {
		if(errno == EAGAIN)
			return SSockets_RET_READ;
		else
			return SSockets_RET_ERROR;
	} else {
		data->ctr = r;
		data->fullctr += r;

		if(!data->fullctr)
			data->conn.send("43 zodiac: empty response from backend\r\n");
		if(!data->ctr)
			return SSockets_RET_FINISHED; // Our job is done

		ctx->state = STATE_PROXY;
		return SSockets_RET_OK;
	}
}
