#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_method() {
	const char *method = getenv("REQUEST_METHOD");
	if(strcasecmp(method, "GET") == 0) {
		return 1;
	} else if(strcasecmp(method, "POST") == 0) {
		return 2;
	} else if(strcasecmp(method, "HEAD") == 0) {
		return 3;
	}
	return 0;
}

void parser(const char *request, int &a, int &b) {
	a = 0;b = 0;
	while(*request != '\0' && *request != '=') {
		++request;
	}
	if(*request == '\0') {
		return;
	}
	++request;
	a = atoi(request);
	while(*request != '\0' && *request != '=') {
		++request;
	}
	if(*request == '\0') {
		return;
	}
	++request;
	b = atoi(request);
}

int main() {
	int method = get_method();
	printf("Content-Type: text/html\n");
	printf("\n");
	if(method == 3) {
		return 0;
	}
	printf("<html><head><title>Calculator</title></head>");
	printf("<body><h1>Calculator</h1>");
	int a = 0, b = 0;
	if(method == 1) {
		const char *request = getenv("QUERY_STRING");
		parser(request, a, b);
	} else if(method == 2) {
		char request[1024] = { 0 }, *p = request;
		int n = atoi(getenv("CONTENT_LENGTH"));
		while(n--) {
			scanf("%c", p);
			++p;
		}
		*p = '\0';
		parser(request, a, b);
	}
	printf("<p>%d + %d = %d</p>", a, b, a + b);
	printf("</body></html>");
	return 0;
}