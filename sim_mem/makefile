app: main.o sim_mem.o
	g++ main.o sim_mem.o -o app

main.o: main.cpp sim_mem.h
	g++ -c main.cpp

sim_mem.o: sim_mem.h sim_mem.cpp
	g++ -c sim_mem.cpp
