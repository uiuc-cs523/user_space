.PHONY: all

all: observable_memory.cc subprocess_a.cc main.cc
	g++ -o test observable_memory.cc subprocess_a.cc main.cc -I.
