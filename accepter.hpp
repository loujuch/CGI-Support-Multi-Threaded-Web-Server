#ifndef _ACCEPTER_HPP__
#define _ACCEPTER_HPP__

#include "scheduler.hpp"

#include <stdint.h>

class Accepter {
	Scheduler scheduler_;
	SOCKET accept_socket_;
public:
	Accepter(uint16_t port, int max_connects);
	~Accepter();

	int run();

	Accepter(const Accepter &) = delete;
	Accepter &operator=(const Accepter &) = delete;
};

#endif // _ACCEPTER_HPP__