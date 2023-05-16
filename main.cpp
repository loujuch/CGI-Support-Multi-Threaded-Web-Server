#include "accepter.hpp"
#include "log.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#pragma comment(lib, "ws2_32.lib")

bool init_lib();
void close_lib();

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Usage: httpd.exe <max_connections>\n");
		return 0;
	}
	int max_connections = atoi(argv[1]);
	if(max_connections <= 0) {
		printf("Usage: httpd.exe <max_connections>\n");
		printf("\tMax Connections must greater than zero.\n");
		return 0;
	}
	assert(init_lib());
	Log::set_log_path("./webroot/log/web.log");
	uint16_t port = 8888;
	Accepter accepter(port, max_connections);
	int res = accepter.run();
	Log::close();
	close_lib();
	return res;
}

bool init_lib() {
	WORD sw_version = MAKEWORD(2, 2);
	WSADATA ws_data;
	return WSAStartup(sw_version, &ws_data) == 0;
}

void close_lib() {
	WSACleanup();
}