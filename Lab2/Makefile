CXXFLAGS+=-O2 -ggdb -DDEBUG
CXXFLAGS+=-Wall -Wextra

all: httpserver

httpserver:global.h queue_pool.cpp server.cpp TCPServer.cpp TCPClient.cpp

	g++  -O2 -o $@ $^ -lpthread

clean: 
	rm httpserver