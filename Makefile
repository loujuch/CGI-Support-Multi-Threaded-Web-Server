OBJ=main.o httpd.o tcp_channel.o accepter.o scheduler.o log.o
CC=g++ -std=c++11
LIB=-lpthread -lws2_32

server: ${OBJ}
	${CC} $^ -o $@ ${LIB}

main.o: main.cpp
	${CC} -c $< -o $@

%.o: %.cpp %.hpp
	${CC} -c $< -o $@

.PHONY:clean
clean:
	-del *.o server.exe