#include <common.hpp>
#include "tasks.hpp"
#include <unistd.h>
#include <sys/mman.h>

size_t pageSize = 0;

int task00init(SSockets_ctx* ctx) {
	if(!pageSize)
		pageSize = sysconf(_SC_PAGE_SIZE);

	Data* data = new Data;
	ctx->data = data;

	data->conn = {ctx->fd, ctx->addr};
	data->conn.createSSL();
	data->bufferSize = pageSize;
	data->buffer = (char*)mmap(0, pageSize, PROT_READ | PROT_WRITE,
							   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	ctx->timeout = globalConfig.hsTimeout;
	ctx->state = STATE_HANDSHAKE;
	return SSockets_RET_OK;
}
