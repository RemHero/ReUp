CXXFLAGS+=-O2 -ggdb -DDEBUG
CXXFLAGS+=-Wall -Wextra

all: sudoku

sudoku: neighbor.cc sudoku_basic.cc sudoku_dancing_links.cc sudoku_min_arity.cc sudoku_min_arity_cache.cc queue_pool.cpp main.cpp

	g++  -O2 -o $@ $^ -lpthread
