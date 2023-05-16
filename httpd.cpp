#include "httpd.hpp"
#include "log.hpp"

#include <map>
#include <fstream>

#include <ctype.h>
#include <assert.h>
#include <time.h>
#include <sys/stat.h>

const char method_string[4][10] = { "UNKNOWN", "GET", "POST", "HEAD" };

enum EXEC_METHOD {
	NO_EXEC = 0,
	PY
};

enum HTTP_METHOD {
	UNKNOWN = 0,
	GET,
	POST,
	HEAD
};

HTTP_METHOD parser_method(const char *method) {
	if(strcasecmp(method, "GET") == 0) {
		return GET;
	} else if(strcasecmp(method, "POST") == 0) {
		return POST;
	} else if(strcasecmp(method, "HEAD") == 0) {
		return HEAD;
	}
	return UNKNOWN;
}

bool parser_request_line(char *request_line, int len, HTTP_METHOD &method,
	std::string &path, std::string &query_string) {
	bool is = false;
	char *p = request_line;
	while(*p != ' ' && *p != '\0') {
		++p;
	}
	if(*p == '\0') {
		return false;
	}
	*p++ = '\0';
	method = parser_method(request_line);
	request_line = p;
	while(*p != ' ' && *p != '\0' && *p != '?') {
		++p;
	}
	if(*p == '\0') {
		return false;
	}
	is = (*p == '?');
	*p++ = '\0';
	path = request_line;
	request_line = p;
	if(is) {
		while(*p != ' ' && *p != '\0') {
			++p;
		}
		if(*p == '\0') {
			return false;
		}
		*p = '\0';
		query_string = request_line;
	}
	return !path.empty();
}

bool parser_request_body(std::map<std::string, std::string> &request_body, char *request_body_str) {
	char *key = request_body_str;
	while(*request_body_str != ':' && *request_body_str != '\0') {
		++request_body_str;
	}
	if(*request_body_str == '\0') {
		return false;
	}
	*request_body_str++ = '\0';
	if(*request_body_str != ' ') {
		return false;
	}
	++request_body_str;
	if(*request_body_str == '\0') {
		return false;
	}
	request_body.emplace(key, request_body_str);
	return true;
}

void bad_request(TCPChannel &tcp) {
	int size = 0, n = 0;
	char buffer[1024];
	n = snprintf(buffer, sizeof(buffer) - size, "HTTP/1.0 400 Bad Request\r\n");
	assert(n > 0);
	size += n;
	n = snprintf(buffer + size, sizeof(buffer) - size, "Content-Type: text/html\r\n");
	assert(n > 0);
	size += n;
	n = snprintf(buffer + size, sizeof(buffer) - size, "\r\n");
	assert(n > 0);
	size += n;
	n = snprintf(buffer + size, sizeof(buffer) - size, "<html><head><title>400</title></head>"
		"<body><h1>Bad Request</h1><p>The server cannot understand your request!</p></body></html>");
	assert(n > 0);
	size += n;
	tcp.write(buffer, size);
}

void unimplemented(TCPChannel &tcp) {
	int size = 0, n = 0;
	char buffer[1024];
	n = snprintf(buffer, sizeof(buffer) - size, "HTTP/1.0 501 Method Not Implemented\r\n");
	assert(n > 0);
	size += n;
	n = snprintf(buffer + size, sizeof(buffer) - size, "Content-Type: text/html\r\n");
	assert(n > 0);
	size += n;
	n = snprintf(buffer + size, sizeof(buffer) - size, "\r\n");
	assert(n > 0);
	size += n;
	n = snprintf(buffer + size, sizeof(buffer) - size, "<html><head><title>503</title></head>"
		"<body><h1>Method Not Implemented</h1>"
		"<p>The server was requested with no implemented method!</p></body></html>");
	assert(n > 0);
	size += n;
	tcp.write(buffer, size);
}

void send_file(TCPChannel &tcp, const std::string &path) {
	char buffer[1024];
	std::ifstream file(path, std::ios::in | std::ios::binary);
	assert(file.is_open());
	while(!file.eof()) {
		file.read(buffer, sizeof(buffer));
		size_t size = file.gcount();
		ssize_t n = tcp.write(buffer, size);
		if(n <= 0) {
			break;
		}
	}
	file.close();
}

void no_found_file(TCPChannel &tcp, HTTP_METHOD method) {
	int size = 0, n = 0;
	char buffer[1024];
	n = snprintf(buffer, sizeof(buffer) - size, "HTTP/1.0 404 Not Found\r\n");
	assert(n > 0);
	size += n;
	n = snprintf(buffer + size, sizeof(buffer) - size, "Content-Type: text/html\r\n");
	assert(n > 0);
	size += n;
	n = snprintf(buffer + size, sizeof(buffer) - size, "\r\n");
	assert(n > 0);
	size += n;
	ssize_t m = tcp.write(buffer, size);
	if(method == HEAD || m <= 0) {
		return;
	}
	send_file(tcp, "./webroot/404.html");
}

std::string get_type(const std::string &s) {
	if(s == "html") {
		return "text/html";
	}
	if(s == "jpg" || s == "jpeg") {
		return "image/jpeg";
	}
	return "text/html";
}

bool send_ok_header(TCPChannel &tcp, const std::string &path) {
	int n = path.size() - 1;
	for(;n >= 0;--n) {
		if(path[n] == '.') {
			break;
		}
	}
	std::string type;
	if(n < 0) {
		type = "test/html";
	} else {
		type = get_type(path.substr(n + 1));
	}
	int size = 0;
	n = 0;
	char buffer[1024];
	n = snprintf(buffer, sizeof(buffer) - size, "HTTP/1.0 200 OK\r\n");
	assert(n > 0);
	size += n;
	n = snprintf(buffer + size, sizeof(buffer) - size, "Content-Type: %s\r\n\r\n", type.c_str());
	assert(n > 0);
	size += n;
	ssize_t m = tcp.write(buffer, size);
	return m > 0;
}

void execute_cgi(TCPChannel &tcp, HTTP_METHOD method, const std::string &path,
	const std::string &query_string, const std::map<std::string, std::string> &request_body, Log &log) {
	int content_length = 0;
	if(method == GET) {
	} else if(method == POST) {
		auto p = request_body.find("Content-Length");
		if(p == request_body.end()) {
			bad_request(tcp);
			return;
		}
		content_length = atoi(p->second.c_str());
	} else if(method == HEAD) {
	} else {
		return;
	}

	HANDLE hParentRead, hParentWrite, hChildRead, hChildWrite;
	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = { 0 };

	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;

	assert(CreatePipe(&hParentRead, &hChildWrite, &sa, 0));
	assert(CreatePipe(&hChildRead, &hParentWrite, &sa, 0));

	si.hStdInput = hChildRead;
	si.hStdOutput = hChildWrite;
	si.dwFlags = STARTF_USESTDHANDLES;

	char buffer[1024], cgi_path[256];
	strncpy(cgi_path, path.c_str(), sizeof(cgi_path));

	Httpd::mutex_.lock();
	snprintf(buffer, sizeof(buffer), "REQUEST_METHOD=%s", method_string[method]);
	putenv(buffer);
	if(method == GET) {
		snprintf(buffer, sizeof(buffer), "QUERY_STRING=%s", query_string.c_str());
		putenv(buffer);
	} else if(method == POST) {
		snprintf(buffer, sizeof(buffer), "CONTENT_LENGTH=%d", content_length);
		putenv(buffer);
	}
	BOOL is_process = CreateProcess(nullptr, cgi_path, NULL, NULL, TRUE,
		CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	Httpd::mutex_.unlock();

	if(is_process == FALSE) {
		printf("ERROR: %d\n", GetLastError());
		return;
	}
	CloseHandle(hChildRead);
	CloseHandle(hChildWrite);

	if(method == POST) {
		DWORD size = content_length;
		while(content_length != 0) {
			ssize_t n = sizeof(buffer) > content_length ? content_length : sizeof(buffer);
			n = tcp.read(buffer, n);
			if(n <= 0) {
				TerminateProcess(pi.hProcess, -1);
				return;
			}
			WINBOOL is = WriteFile(hParentWrite, buffer, n, &size, 0);
			if(is == FALSE) {
				printf("FALSE: %d\n", GetLastError());
			}
			// printf("%d\n", size);
			// printf("%s\n", buffer);
			content_length -= n;
		}
	}

	WINBOOL test;
	DWORD size = snprintf(buffer, sizeof(buffer), "HTTP/1.0 200 OK\r\n"), tmp = size;
	log.log_append("200");
	log.log_append("CGI");
	while(true) {
		test = ReadFile(hParentRead, buffer + tmp, sizeof(buffer) - tmp, &size, 0);
		if(test == FALSE) {
			break;
		}
		int n = tcp.write(buffer, tmp + size);
		if(n <= 0) {
			TerminateProcess(pi.hProcess, -1);
			return;
		}
		buffer[tmp + size] = '\0';
		tmp = 0;
	}

	CloseHandle(hParentRead);
	CloseHandle(hParentWrite);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	WaitForSingleObject(pi.hProcess, 3000);
}

void reply(TCPChannel &tcp, HTTP_METHOD method, const std::string &path,
	const std::string &query_string, const std::map<std::string, std::string> &request_body, Log &log) {
	struct stat stat_struct;
	int n = stat(path.c_str(), &stat_struct);
	if(n != 0) {
		log.log_append("404 -");
		no_found_file(tcp, method);
		return;
	}
	if(method == POST || !query_string.empty() || path.find("cgi-bin") != std::string::npos) {
		execute_cgi(tcp, method, path, query_string, request_body, log);
	} else {
		log.log_append("200");
		log.log_append(std::to_string(stat_struct.st_size));
		bool is = send_ok_header(tcp, path);
		if(!is || method == HEAD) {
			return;
		}
		send_file(tcp, path);
	}
}

std::string Httpd::path_root = "./webroot";
std::mutex Httpd::mutex_;

void Httpd::http_server(TCPChannel &tcp, const sockaddr_in addr) {
	ssize_t n;
	bool vaild = true;
	HTTP_METHOD method = UNKNOWN;
	char buffer[1024] = { 0 };
	std::map<std::string, std::string> request_body;
	std::map<std::string, std::string> reply_body;
	std::string path(""), query_string("");
	Log log;
	log.log_append(inet_ntoa(addr.sin_addr));
	log.log_append("- -");
	time_t t = time(nullptr);
	struct tm *tt = gmtime(&t);
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tt);
	reply_body.emplace("Data", buffer);
	log.log_append("[" + std::string(buffer) + "]");
	do {
		n = tcp.read_line(buffer, sizeof(buffer));
		// printf("%s\n", buffer);
		if(n <= 0) {
			break;
		}
		log.log_append("\"" + std::string(buffer) + "\"");
		vaild &= parser_request_line(buffer, n, method, path, query_string);
		if(path.back() == '/') {
			path += "index.html";
		}
		if(path[0] != '/') {
			path = path_root + "/" + path;
		} else {
			path = path_root + path;
		}
		n = tcp.read_line(buffer, sizeof(buffer));
		// printf("%s\n", buffer);
		if(n < 0) {
			break;
		}
		while(n > 0) {
			vaild &= parser_request_body(request_body, buffer);
			n = tcp.read_line(buffer, sizeof(buffer));
			// printf("%s\n", buffer);
		}
		if(n < 0) {
			break;
		}
		if(!vaild) {
			log.log_append("400 -");
			bad_request(tcp);
		} else if(method == UNKNOWN) {
			log.log_append("501 -");
			unimplemented(tcp);
		} else {
			reply(tcp, method, path, query_string, request_body, log);
		}
		if(request_body.count("Referer") != 0) {
			log.log_append("\"" + request_body["Referer"] + "\"");
		} else {
			log.log_append("-");
		}
		if(request_body.count("User-Agent") != 0) {
			log.log_append("\"" + request_body["User-Agent"] + "\"");
		} else {
			log.log_append("-");
		}
	} while(false);
	log.log_post();
	tcp.close();
}