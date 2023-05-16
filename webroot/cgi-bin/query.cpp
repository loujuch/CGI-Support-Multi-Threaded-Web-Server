#include <fstream>

#include <stdio.h>
#include "mysql.h"
#pragma comment(lib, "libmysql.lib")

void query_mysql(const char *id) {
	MYSQL *conn;
	conn = mysql_init((MYSQL *)NULL);
	if(conn == NULL) {
		printf("mysql_init\n");
		return;
	}

	if(NULL == mysql_real_connect(conn, "localhost", "root", "11010939@Ch", "student_info", 3306, NULL, 0)) {
		printf("mysql_real_connect: %s\n", mysql_error(conn));
		mysql_close(conn);
		return;
	}

	char buffer[1024];
	int n = snprintf(buffer, sizeof(buffer), "SELECT id, name, class FROM student WHERE id = \"%s\"", id);
	if(0 != mysql_real_query(conn, buffer, n)) {
		printf("mysql_real_query: %s\n", mysql_error(conn));
		mysql_close(conn);
		return;
	}

	MYSQL_RES *res = NULL;
	res = mysql_store_result(conn);
	if(res == NULL) {
		printf("mysql_store_result: %s\n", mysql_error(conn));
		mysql_close(conn);
		return;
	}

	MYSQL_ROW row;
	while((row = mysql_fetch_row(res))) {
		printf("<p><b>ID</b>: %s, <b>Name</b>: %s, <b>Class</b>: %s</p>", row[0], row[1], row[2]);
	}
	mysql_close(conn);
}

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

int main() {
	int method = get_method();
	printf("Content-Type: text/html\n");
	printf("\n");
	if(method == 3) {
		return 0;
	}
	printf("<html><head><title>Query</title></head>");
	printf("<body><h1>Query</h1>");

	if(method == 1) {
		const char *request = getenv("QUERY_STRING");
		query_mysql(request);
	} else if(method == 2) {
		bool is = false;
		char request[1024] = { 0 }, c;
		int n = atoi(getenv("CONTENT_LENGTH")), m = 0;

		while(n--) {
			scanf("%c", &c);
			if(is) {
				request[m++] = c;
			} else {
				is = (c == '=');
			}
		}
		request[m] = '\0';
		query_mysql(request);
	}

	printf("</body></html>");
	return 0;
}