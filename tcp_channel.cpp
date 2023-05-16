#include "tcp_channel.hpp"

#include <assert.h>

TCPChannel::TCPChannel(SOCKET socket):is_close_(false), tcp_socket_(socket) {
	assert(tcp_socket_ != INVALID_SOCKET);
}

TCPChannel::~TCPChannel() {
	assert(close());
}

ssize_t TCPChannel::read_line(char *buffer, size_t size) {
	bool is = false;
	char c = '\n';
	ssize_t i = 0, tmp;
	for(i = 0;i + 1 < size;++i) {
		tmp = read(&c, 1);
		if(tmp <= 0) {
			return -1;
		}
		if(tmp != 0 && !is) {
			if(c == '\r') {
				--i;
				is = true;
				continue;
			}
			buffer[i] = c;
		} else {
			break;
		}
	}
	buffer[i] = '\0';
	return i;
}

ssize_t TCPChannel::read(char *buffer, size_t size) {
	// ssize_t n = recv(tcp_socket_, buffer, size, 0);
	// buffer[size] = '\0';
	return recv(tcp_socket_, buffer, size, 0);
}

ssize_t TCPChannel::write(const char *buffer, size_t size) {
	// printf("%s\n", buffer);
	return send(tcp_socket_, buffer, size, 0);
}

int TCPChannel::close() {
	mutex_.lock();
	if(is_close_) {
		return true;
	}
	int n = closesocket(tcp_socket_);
	is_close_ = (0 == n);
	mutex_.unlock();
	return is_close_;
}