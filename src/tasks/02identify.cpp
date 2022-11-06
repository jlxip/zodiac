#include <common.hpp>
#include "tasks.hpp"

int task02identify(SSockets_ctx* ctx) {
	/*
		TLS handshake has just finished. Now, we wait for the path.
		Let's first get the server name for two reasons:
		  i) "Doom" the connection to fail in case it's invalid and there's
		       no default. It can't fail right now, the response MUST be
		       sent after the request. Clients don't like it any other way.
		  ii) Set the proper timeout for the read.
	*/

	Data* data = (Data*)(ctx->data);

	// Get server name (and a pointer to CapsuleConfig if possible)
	auto it = globalServer.names.find(data->conn.getSN());
	if(it == globalServer.names.end()) {
		// Unknown. Is default explicitly set?
		if(globalConfig.hasExplicitDefault) {
			// Yep, no worries
			data->capsule = &(globalConfig.capsules[globalConfig.def]);
		} else {
			// No; doom the connection
			data->doomed = true;
		}
	} else {
		// Nice
		data->capsule = &(globalConfig.capsules[(*it).second]);
	}

	// Set the frontend timeout
	ctx->timeout = globalConfig.frontTimeout;
	if(!data->doomed) {
		// If it's not doomed, then there's a capsule attached, so let's
		//   set its timeout, in case there is one.
		size_t specific = data->capsule->frontTimeout;
		if(specific)
			ctx->timeout = specific;
	}

	ctx->state = STATE_RECEIVE;
	return SSockets_RET_OK;
}
