#ifndef TASKS_HPP
#define TASKS_HPP

enum {
	STATE_INIT,
	STATE_HANDSHAKE,
	STATE_IDENTIFY,
	STATE_RECEIVE,
	STATE_PREPARE_CONNECT,
	STATE_CONNECT,

	STATE_PREPARE_BACKIN,
	STATE_BACKIN,
	STATE_PREPARE_BACKOUT,
	STATE_BACKOUT,
	STATE_PROXY,
};

int task00init(SSockets_ctx* ctx);
int task01handshake(SSockets_ctx* ctx);
int task02identify(SSockets_ctx* ctx);
int task03receive(SSockets_ctx* ctx);
int task04prepareConnect(SSockets_ctx* ctx);
int task05connect(SSockets_ctx* ctx);

int task06prepareBackin(SSockets_ctx* ctx);
int task07backin(SSockets_ctx* ctx);
int task08prepareBackout(SSockets_ctx* ctx);
int task09backout(SSockets_ctx* ctx);
int task10proxy(SSockets_ctx* ctx);

#endif
