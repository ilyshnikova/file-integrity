#ifndef DAEMONS
#define DAEMONS

#include <string>

#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include "exception.h"

/*	LibSocket	*/

class LibSocket {
public:

	std::string ip;
	std::string port;
	struct addrinfo host_info;
	struct addrinfo * host_info_list;

	LibSocket(const std::string& ip, const std::string& port);


	void Prepare();

	int Start();

	std::string GetMessage(const size_t RECV_PART, struct timeval tv, const int socketfd) const;

	void SendMessage(const int socketfd, const std::string& send_message) const;



};

/*	BaseClient	*/

class BaseClient : public LibSocket {
private:
	int Connect();

	virtual void Callback(const std::string& answer) const;

	virtual bool GetQuery(std::string* query);

	virtual std::string CreateQueryForDaemon(const std::string& query) const;

	virtual std::string CreateAnswer(const std::string& json);
public:
	bool Conversation(std::string* answer, const size_t RECV_PART, struct timeval tv);

	BaseClient(const std::string& ip, const std::string& port);

	void Process();
};


/*	TerminalClient	*/

class TerminalClient : public BaseClient {
private:


public:
	TerminalClient(const std::string& ip, const std::string& port);
};



/*	DaemonBase	*/

class DaemonBase : public LibSocket {
private:
	int Connect();

	virtual std::string Respond(const std::string& query);

public:

	DaemonBase(const std::string& ip, const std::string& port);

	DaemonBase(const std::string& ip, const std::string& port, const int option);

	void Daemon();
};

/*	EchoDaemon	*/

class EchoDaemon : public DaemonBase {
private:
	std::string Respond(const std::string& query);

public:
	EchoDaemon(const std::string& ip, const std::string& port);

};

#endif

