#include <common.hpp>
#include "tasks.hpp"

int task01handshake(SSockets_ctx* ctx) {
	Data* data = (Data*)(ctx->data);
	if(!data->conn.checkHandshake()) {
		auto r = data->conn.doHandshake();
		if(r == SSockets_RET_OK)
			ctx->state = STATE_IDENTIFY;
		return r;
	}

	ctx->state = STATE_IDENTIFY;
	return SSockets_RET_OK;
}
