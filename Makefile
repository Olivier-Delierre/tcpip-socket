CXX=gcc
FLAGS=-lpthread
SRC=src/

all: tcpCHAT server

#=======================================

tcpCHAT: client.o
	$(CXX) client.o -o tcpCHAT $(FLAGS)

server: server.o
	$(CXX) server.o -o server $(FLAGS)

#========================================

client.o: $(SRC)client.c
	$(CXX) -o client.o -c $(SRC)client.c $(FLAGS)

server.o: $(SRC)server.c
	$(CXX) -o server.o -c $(SRC)server.c $(FLAGS)

#========================================

clean:
	@rm -f *.o

mrproper:
	@rm -f *.o tcpCHAT server
