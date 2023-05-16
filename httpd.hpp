#ifndef _HTTPD_HPP__
#define _HTTPD_HPP__

#include <string>
#include <mutex>

#include "tcp_channel.hpp"

#include <stdint.h>

class Httpd {
public:
	static std::mutex mutex_;
	static std::string path_root;

	static void http_server(TCPChannel &tcp, const sockaddr_in addr);

	Httpd(TCPChannel &) = delete;
	Httpd(const Httpd &) = delete;
	Httpd &operator=(const Httpd &) = delete;
};

#endif // _HTTPD_HPP__