#include <openssl/md5.h>
#include <fstream>

#include "BerkeleyDB.h"
#include "daemon.h"
#include "file-integrity.h"
#include "exception.h"
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <regex>


/*	WorkSpace	*/

WorkSpace::WorkSpace()
: DaemonBase("127.0.0.1", "8081", 0)
, checksum_table("CheckSums.db", "./db")
{
	if (!fork()) {
		Daemon();
		wait(NULL);
	} else {
		RecCheck();
	}
}

std::string WorkSpace::Respond(const std::string& query) {
	const char* cquery = query.c_str();
	std::cmatch match;

	try {
		if (
			std::regex_match(cquery, match, std::regex("\\s*"))
		) {
			return query;
		} else if (
			std::regex_match(cquery, match, std::regex("check\\s+file\\s+(.+)"))

		) {
			if (access(std::string(match[1]).c_str(), F_OK ) == -1) {
				throw FIException(142142, "File with name " + std::string(match[1]) + " does not exits.");
			}
			if (CheckFile(match[1])) {
				return "File hasn't changed.";
			} else {
				return "File has changed.";
			}
		} else if (
			std::regex_match(cquery, match, std::regex("add\\s+file\\s+(.+)"))

		) {
			if (access(std::string(match[1]).c_str(), F_OK ) == -1) {
				throw FIException(171142, "File with name " +std::string(match[1]) + " does not exits.");
			}

			AddCheckSum(match[1]);
		} else if (
			std::regex_match(cquery, match, std::regex("delete\\s+file\\s+(.+)"))
	 	) {
			checksum_table.Delete(std::string(match[1]));
		} else if (
			std::regex_match(cquery, match, std::regex("update\\s+file\\s+(.+)"))
		) {
			if (access(std::string(match[1]).c_str(), F_OK ) == -1) {
				throw FIException(171142, "File with name " + std::string(match[1]) + " does not exits.");
			}

			AddCheckSum(match[1]);
			return "Ok";

		} else if (
			std::regex_match(cquery, match, std::regex("\\s*help\\s*"))
		) {
			return
				std::string("Queries:\n")
				+"\tadd file <filename>\n"
				+ "\tdelete file <filename>\n"
				+ "\tcheck file <filname>\n"
				+ "\tupdate file <filename>";
		} else {
			return "Incorrent query.";
		}
	} catch (std::exception& e) {
		return e.what();
	}

	return "Ok";
}



std::string WorkSpace::Evaluation(const std::string& string) const {
	const char* buffer = string.c_str();
	unsigned char digest[MD5_DIGEST_LENGTH];
	MD5((unsigned char*)buffer, string.size(), (unsigned char*)&digest);
	char cur_cstr[33];

	for(int i = 0; i < 16; ++i) {
		sprintf(&cur_cstr[i*2], "%02x", (unsigned int)digest[i]);
	}
	cur_cstr[32] = '\0';

	return std::string(cur_cstr);

}


std::string WorkSpace::GetAllFile(const std::string& file_name) const {
	std::string file;
	std::ifstream is (file_name);
	if (is) {
		is.seekg (0, is.end);
		int length = is.tellg();
		is.seekg (0, is.beg);

		char buffer[length];
		is.read (buffer,length);

		if (!is) {
			throw FIException(144522, "Error read file.");
		}
		is.close();

		buffer[length] =  '\0';

		file = std::string(buffer);

	}

	logger << "\"" +  file  + "\"";

	return file;
}


std::string WorkSpace::FilesEvaluation(const std::string& file_name) const {
	std::string res = Evaluation((GetAllFile(file_name)));
	logger << res;
	return res;
}

void WorkSpace::AddCheckSum(const std::string& file_name) {
	std::string sum = FilesEvaluation(file_name);
	checksum_table.Insert(file_name, sum);
}


bool WorkSpace::CheckFile(const std::string& file_name) const {
	return (FilesEvaluation(file_name) == std::string(checksum_table.Select(file_name)));
}


void WorkSpace::RecCheck() {
	while (true) {
		for (auto it = checksum_table.begin(); it != checksum_table.end(); ++it) {
			if (!CheckFile(it.Key())) {
				std::string message = "File " + std::string(it.Key()) + " has changed.";
				std::string mail = "echo \"" + message + "\" | mail -s \"file-integrity\" \"ilyshnikova@yandex.ru\"";
				logger << mail;
				system(mail.c_str());
				logger << std::string("!!!!  ")	+ std::string(it.Key())  + "  changed !!!!";

			} else {
				logger << std::string(it.Key())  + " ok";
			}
			AddCheckSum(std::string(it.Key()));
		}

			sleep(5);
	}
}


