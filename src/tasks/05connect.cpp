#include <common.hpp>
#include "tasks.hpp"

int task05connect(SSockets_ctx* ctx) {
	Data* data = (Data*)(ctx->data);

	// Connect now
	auto r = connect(data->backend, (struct sockaddr*)&(data->capsule->saddr),
					 sizeof(sockaddr));

	if(r == 0) {
		ctx->state = STATE_PREPARE_BACKIN;
		return SSockets_RET_OK;
	}

	switch(errno) {
	case EINPROGRESS:
	case EAGAIN:
		return SSockets_RET_WRITE;
	default:
		// Failed :/
		data->conn.send("43 zodiac: could not connect to backend\r\n");
		return SSockets_RET_ERROR;
	}
}
