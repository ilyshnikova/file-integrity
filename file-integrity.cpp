#include <openssl/md5.h>
#include <fstream>

#include <boost/regex.hpp>
#include "mysql.h"
#include "daemon.h"
#include "file-integrity.h"
#include "exception.h"


/*	WorkSpace	*/

WorkSpace::WorkSpace()
: DaemonBase("127.0.0.1", "8081", 0)
, checksum_table("CheckSums|FileName:string|CheckSum:string")
{
	Daemon();
}

std::string WorkSpace::Respond(const std::string& query) {

	boost::smatch match;

	try {
		if (
			boost::regex_match(
				query,
				match,
				boost::regex("\\s*")
			)
		) {
			return query;
		} else if (
			boost::regex_match(
				query,
				match,
				boost::regex("check\\s+file\\s+(.+)")
			)
		) {
			if (CheckFile(match[1])) {
				return "File hasn't changed.";
			} else {
				return "File has changed.";
			}
		} else if (
			boost::regex_match(
				query,
				match,
				boost::regex("add\\s+file\\s+(.+)")
			)
		) {
			AddCheckSum(match[1]);
			return "Ok";
		} else {
			return "Incorrent query."
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

		char * buffer = new char [length];
		is.read (buffer,length);

		if (!is) {
			throw FIException(144522, "Error read file.");
		}
		is.close();

		buffer[length] =  '\0';

		file = std::string(buffer);

		delete[] buffer;
	}

	logger << "\"" +  file  + "\"";

	return file;
}


std::string WorkSpace::FilesEvaluation(const std::string& file_name) const {
	std::string res = Evaluation((GetAllFile(file_name)));
	std::cout << res << std::endl;
	return res;
}

void WorkSpace::AddCheckSum(const std::string& file_name) {
	std::string sum = FilesEvaluation(file_name);
	checksum_table.Insert(file_name, sum);
	checksum_table.Execute();
}


bool WorkSpace::CheckFile(const std::string& file_name) {
	auto file_info = checksum_table.Select("FileName = \"" + file_name + "\"");
	if (file_info == checksum_table.SelectEnd()) {
		throw FIException(256247, "This file is not under control.");
	}
	std::string last_sum = file_info["CheckSum"];
	std::string current_sum;

	return FilesEvaluation(file_name) == std::string(file_info["CheckSum"]);
}
