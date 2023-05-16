#ifndef _TCP_CHANNEL_HPP__
#define _TCP_CHANNEL_HPP__

#include <mutex>

#include <stdint.h>
#include <winsock2.h>

class TCPChannel {
	bool is_close_;
	SOCKET tcp_socket_;
	std::mutex mutex_;
public:
	TCPChannel(SOCKET socket);
	~TCPChannel();

	ssize_t read_line(char *buffer, size_t size);

	ssize_t read(char *buffer, size_t size);
	ssize_t write(const char *buffer, size_t size);

	int close();

	TCPChannel(const TCPChannel &) = delete;
	TCPChannel &operator=(const TCPChannel &) = delete;
};

#endif // _TCP_CHANNEL_HPP__