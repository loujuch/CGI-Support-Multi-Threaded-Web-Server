#include "scheduler.hpp"

#include <thread>

#include "httpd.hpp"

Scheduler::Scheduler(int max_connects):
	max_connects_(max_connects) {
}

void Scheduler::new_work(SOCKET sock, const sockaddr_in &addr) {
	mutex_.lock();
	if(tcp_channel_.size() >= max_connects_) {
		tcp_channel_.front().close();
		tcp_channel_.pop_front();
	}
	tcp_channel_.emplace_back(sock);
	std::thread tmp(Httpd::http_server, std::ref(tcp_channel_.back()), addr);
	tmp.detach();
	mutex_.unlock();
}