CXXFLAGS+=-O2 -ggdb -DDEBUG
CXXFLAGS+=-Wall -Wextra

all: sudoku_solve

vpath %.cc algorithm
vpath %.h algorithm
vpath %.hpp fileReader
vpath %.cpp thread_pool
vpath %.h thread_pool

sudoku_solve: neighbor.cc sudoku_basic.cc sudoku_dancing_links.cc sudoku_min_arity.cc sudoku_min_arity_cache.cc queue_pool.cpp fileReader.hpp main.cpp

	g++  -O2 -o $@ $^ -lpthread

clean: 
	rm sudoku_solve
