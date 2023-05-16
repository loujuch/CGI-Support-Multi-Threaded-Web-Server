#include "log.hpp"

std::fstream Log::file_stream_;
std::string Log::log_path_("./webroot/log/web.log");
std::mutex Log::mutex_;

void Log::log_append(const std::string &s) {
	if(!str.empty()) {
		str.push_back(' ');
	}
	str += s;
}

void Log::log_post() {
	if(!file_stream_.is_open()) {
		return;
	}
	mutex_.lock();
	file_stream_ << str << '\n';
	mutex_.unlock();
}

void Log::set_log_path(const std::string &s) {
	if(file_stream_.is_open()) {
		file_stream_.close();
	}
	log_path_ = s;
	file_stream_.open(log_path_, std::ios::out);
}

void Log::close() {
	if(file_stream_.is_open()) {
		file_stream_.close();
	}
}