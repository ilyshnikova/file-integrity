all: fi-server fi-client
BASEP = -g -Wall -std=c++0x -gdwarf-3

exception.o: exception.cpp
	g++ $(BASEP) -c exception.cpp

logger.o: logger.cpp
	g++ $(BASEP) -c logger.cpp

daemon.o: daemon.cpp
	g++ $(BASEP) -c daemon.cpp

server.o: server.cpp
	g++ $(BASEP) -c server.cpp

fi-server: daemon.o logger.o exception.o server.o
	g++ $(BASEP) daemon.o logger.o exception.o server.o -o server

client: client.cpp
	g++ $(BASEP) -c client.cpp

fi-client: daemon.o logger.o exception.o client.o
	g++ $(BASEP)  daemon.o logger.o exception.o client.o -o client

clean:
	rm -rf *.o server client

