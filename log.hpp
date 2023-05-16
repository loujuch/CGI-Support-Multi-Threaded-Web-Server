#ifndef _LOG_HPP__
#define _LOG_HPP__

#include <string>
#include <fstream>
#include <mutex>

class Log {
	static std::fstream file_stream_;
	static std::string log_path_;
	static std::mutex mutex_;
	std::string str;
public:
	void log_append(const std::string &s);
	void log_post();

	static void set_log_path(const std::string &s);
	static void close();
};

#endif // _LOG_HPP__