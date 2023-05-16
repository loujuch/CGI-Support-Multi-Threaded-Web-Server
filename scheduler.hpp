#ifndef _SCHEDULER_HPP__
#define _SCHEDULER_HPP__

#include <deque>
#include <mutex>

#include "tcp_channel.hpp"

class Scheduler {
	std::mutex mutex_;
	int max_connects_;
	std::deque<TCPChannel> tcp_channel_;
public:
	Scheduler(int max_connects);

	void new_work(SOCKET socket, const sockaddr_in &addr);
};

#endif // _SCHEDULER_HPP__