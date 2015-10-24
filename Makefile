all: fi-server fi-client daemon
BASEP = -g -Wall -std=c++0x -gdwarf-3

exception.o: exception.cpp
	g++ $(BASEP) -c exception.cpp

logger.o: logger.cpp
	g++ $(BASEP) -c logger.cpp

daemon.o: daemon.cpp
	g++ $(BASEP) -c daemon.cpp

server.o: server.cpp
	g++ $(BASEP) -c server.cpp

daemon: daemon.o logger.o exception.o server.o
	g++ $(BASEP) daemon.o logger.o exception.o server.o -o daemon

client: client.cpp
	g++ $(BASEP) -c client.cpp

fi-client: daemon.o logger.o exception.o client.o
	g++ $(BASEP) daemon.o logger.o exception.o client.o -o client

fi-server.o: fi-server.cpp
	g++ $(BASEP) -c fi-server.cpp

mysql.o: mysql.cpp
	g++ $(BASEP) -c mysql.cpp

file-integrity.o: file-integrity.cpp
	g++ $(BASEP) -c file-integrity.cpp

fi-server: fi-server.o exception.o logger.o daemon.o file-integrity.o mysql.o
	g++ $(BASEP) -lboost_regex  -lmysqlcppconn -lcrypto  fi-server.o exception.o logger.o daemon.o file-integrity.o mysql.o -o server

clean:
	rm -rf *.o daemon client server
