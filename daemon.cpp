#include <string>
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include "exception.h"
#include "daemon.h"
#include "logger.h"

/*    LibSocket     */

LibSocket::LibSocket(const std::string& ip, const std::string& port)
: ip(ip)
, port(port)
, host_info()
, host_info_list()
{}



void LibSocket::Prepare() {
	memset(&host_info, 0, sizeof host_info);

	host_info.ai_family = AF_UNSPEC;
	host_info.ai_socktype = SOCK_STREAM;
}


int LibSocket::Start() {
	int status = getaddrinfo(ip.c_str(), port.c_str(), &host_info, &host_info_list);
	if (status != 0) {
		throw FIException(324832, "getaddrinfo failed: " + std::to_string(status));
	}


	int socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
	if (socketfd == -1) {
		throw FIException(235153 , "Creating socket failed");
	}


	return socketfd;
}


std::string LibSocket::GetMessage(const size_t RECV_PART, struct timeval tv, const int socketfd) const {
	std::string got_message = "";
	char * incoming_data_buffer = new char[RECV_PART + 1];

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(socketfd, &fds);

	while (1) {
		memset(incoming_data_buffer, 0, RECV_PART + 1);

		int select_result = select(socketfd + 1, &fds, NULL, NULL, &tv);
		if (select_result < 0) {
			throw FIException(954625, "select failed");
		}

		if (!select_result) {
			break;
		}

		ssize_t bytes_recieved = recv(socketfd, incoming_data_buffer, RECV_PART, 0);
		if (bytes_recieved == 0) {
		       	break;
		}

		got_message += std::string(incoming_data_buffer);
	}
	delete [] incoming_data_buffer;
	return got_message;
}


void LibSocket::SendMessage(const int socketfd, const std::string& send_message) const {
	send(socketfd, send_message.c_str(), send_message.size(), 0);

}

/*    BaseClient      */

int BaseClient::Connect()  {
	int socketfd = Start();

	int status = connect(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
	if (status == -1) {
		throw FIException(247528, "connect failed");
	}

	return socketfd;
}

std::string BaseClient::CreateQueryForDaemon(const std::string& query) const {
	return query;
}

std::string BaseClient::CreateAnswer(const std::string& query) {
	return query;
}




bool BaseClient::GetQuery(std::string* query)  {
	std::cout << "integrity> ";
	if (!std::getline(std::cin, *query)) {
		std::cout << "\n";
		return false;
	}
	return !std::cin.eof();
}


bool BaseClient::Conversation(std::string* answer, const size_t RECV_PART, struct timeval tv) {
	std::string query;
	if (!GetQuery(&query)) {
		return false;
	}
	int socketfd;
	try {
		socketfd = Connect();
		query = CreateQueryForDaemon(query);
		SendMessage(socketfd, query);
		shutdown(socketfd, 1);

		std::string got_message = GetMessage(RECV_PART, tv, socketfd);
		*answer = CreateAnswer(got_message);

	} catch (std::exception& e) {
		*answer =  e.what();
	}

	freeaddrinfo(host_info_list);
	close(socketfd);
	return true;
}

void BaseClient::Callback(const std::string& answer) const {
	std::cout << answer << "\n";
}

void BaseClient::Process() {
	const size_t RECV_PART = 10000;

	double timeout = 900;
	struct timeval tv;
	tv.tv_sec = int(timeout);
	tv.tv_usec = int((timeout - tv.tv_sec) * 1e6);

	Prepare();

	while (1) {
		std::string answer;
		if (!Conversation(&answer, RECV_PART, tv)) {
			return;
		}
		Callback(answer);
	}

}


BaseClient::BaseClient(const std::string& ip, const std::string& port)
: LibSocket(ip, port)
{}



/*	TerminalClient	*/




TerminalClient::TerminalClient(const std::string& ip, const std::string& port)
: BaseClient(ip, port)
{}

/*    DaemonBase      */

int DaemonBase::Connect() {
	int socketfd = Start();


	int status = bind(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
	if (status == -1) {
		throw FIException(649265, "bind failed");
	}

	listen(socketfd, 10);

	return socketfd;

}


std::string DaemonBase::Respond(const std::string& query) {
	return query;
}


DaemonBase::DaemonBase(const std::string& ip, const std::string& port)
: LibSocket(ip, port)
{
	Daemon();
}


DaemonBase::DaemonBase(const std::string& ip, const std::string& port, const int )
: LibSocket(ip, port)
{}


void DaemonBase::Daemon() {
	const size_t RECV_PART = 10000;

	double timeout = 900;
	struct timeval tv;
	tv.tv_sec = int(timeout);
	tv.tv_usec = int((timeout - tv.tv_sec) * 1e6);

	Prepare();

	int socketfd = Connect();

	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);

	while (1) {

		int client_socketfd = accept(socketfd, (struct sockaddr *) &cli_addr, &clilen);
		if (client_socketfd < 0) {
			throw FIException(539154, "Accepting client socket failed");
		}


		std::string query = GetMessage(RECV_PART, tv, client_socketfd);

		shutdown(client_socketfd, 0);

		if (query == "shutdown") {
			SendMessage(client_socketfd, "Ok");
			close(client_socketfd);
			break;
		}



		std::string answer;
		try {
			answer = Respond(query);
		} catch (std::exception& e) {
			answer = e.what();
		}
		logger << "answer = " + answer;

		SendMessage(client_socketfd, answer);

		close(client_socketfd);
	}
}


/*    EchoDaemon     */


std::string EchoDaemon::Respond(const std::string& query) {
	return query;
}


EchoDaemon::EchoDaemon(const std::string& ip, const std::string& port)
: DaemonBase(ip, port, 0)
{
	Daemon();
}


