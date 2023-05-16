#include "accepter.hpp"

#include <assert.h>
#include <string.h>

Accepter::Accepter(uint16_t port, int max_connects):
	accept_socket_(INVALID_SOCKET),
	scheduler_(max_connects) {
	accept_socket_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	assert(accept_socket_ != INVALID_SOCKET);

	sockaddr_in tmp;
	memset(&tmp, 0, sizeof(tmp));
	tmp.sin_family = AF_INET;
	tmp.sin_port = htons(port);
	tmp.sin_addr.s_addr = INADDR_ANY;
	assert(bind(accept_socket_, (sockaddr *)&tmp, sizeof(tmp)) != SOCKET_ERROR);

	assert(listen(accept_socket_, SOMAXCONN) != SOCKET_ERROR);
	printf("port: %u\n", port);
}

Accepter::~Accepter() {
	assert(accept_socket_ != INVALID_SOCKET);
	closesocket(accept_socket_);
}

int Accepter::run() {
	SOCKET  client = INVALID_SOCKET;
	sockaddr_in client_addr;
	int addr_len = sizeof(client_addr);
	while(true) {
		client = accept(accept_socket_, (sockaddr *)&client_addr, &addr_len);
		assert(client != INVALID_SOCKET);
		scheduler_.new_work(client, client_addr);
	}
	return 0;
}