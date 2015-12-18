#include <openssl/md5.h>
#include <fstream>
#include "BerkeleyDB.h"
#include "daemon.h"
#include "file-integrity.h"
#include "exception.h"
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <regex>
#include <sys/types.h>
#include <sys/inotify.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>
#include <atomic>


#include "execute.h"

/*	WorkSpace	*/

WorkSpace::WorkSpace()
: DaemonBase("127.0.0.1", "8081", 0)
, checksum_table("CheckSums.db", "/var/db/FileIntegrity/CheckSums/")
, files_by_ind_table("FilesNamesByInotifysDescriptors.db", "/var/db/FileIntegrity/InotifysDescriptors/")
, ind_by_files_table("InotifysDescriptorsByFilesNames.db", "/var/db/FileIntegrity/InotifysDescriptors/")
, inotify(inotify_init())
, m()
{
	pthread_mutexattr_t a;
	pthread_mutexattr_init(&a);
	pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m, &a);
	pthread_mutexattr_destroy(&a);

	AddFilesToInotify();
	pthread_t id;
	pthread_create(&id, NULL, StatInotify, this);
	Daemon();


//	if (!fork()) {
//		Daemon();
//		wait(NULL);
//	} else {
//		RecCheck();
//	}
}

std::string WorkSpace::Respond(const std::string& query) {
	const char* cquery = query.c_str();
	std::cmatch match;

	try {
		if (
			std::regex_match(
				cquery,
				match,
				std::regex("\\s*")
			)
		) {
			return query;
		} else if (
			std::regex_match(
				cquery,
				match,
				std::regex("\\s*check\\s+file\\s+(.+)\\s*")
			)
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
			std::regex_match(
				cquery,
				match,
				std::regex("\\s*add\\s+file\\s+(.+)\\s*")
			)

		) {
			if (access(std::string(match[1]).c_str(), F_OK ) == -1) {
				throw FIException(171142, "File with name " +std::string(match[1]) + " does not exits.");
			}
			pthread_mutex_lock(&m);
			AddFileToInotify(std::string(match[1]));
//			inotify_add_watch(inotify, std::string(match[1]).c_str(), IN_ALL_EVENTS);
			AddCheckSum(match[1]);
			pthread_mutex_unlock(&m);

		} else if (
			std::regex_match(
				cquery,
				match,
				std::regex("\\s*delete\\s+file\\s+(.+)\\s*")
			)
	 	) {

			pthread_mutex_lock(&m);
			int wd = ind_by_files_table.Select(std::string(match[1]));
			logger << wd;
			ind_by_files_table.Delete(std::string(match[1]));
			files_by_ind_table.Delete(wd);
			checksum_table.Delete(std::string(match[1]));
			inotify_rm_watch(inotify, wd);
			pthread_mutex_unlock(&m);
		} else if (
			std::regex_match(
				cquery,
				match,
				std::regex("\\s*update\\s+file\\s+(.+)\\s*")
			)
		) {
			if (access(std::string(match[1]).c_str(), F_OK ) == -1) {
				throw FIException(171142, "File with name " + std::string(match[1]) + " does not exits.");
			}

			AddCheckSum(match[1]);
			return "Ok";

		} else if (
			std::regex_match(
				cquery,
				match,
				std::regex("\\s*add\\s+files\\s+(.+)\\s*")
			)
		) {
			ExecuteHandler ex(" find / | grep -E \'" + std::string(match[1]) + "\'");
			std::string file_name;
			std::string files;
			while (ex >> file_name) {
				files += file_name + " : " + Respond("add file " + file_name) + "\n";
			}
			return files;

		} else if (
			std::regex_match(
				cquery,
				match,
				std::regex("\\s*help\\s*")
			)
		) {
			return
				std::string("Queries:\n")
				+"\tadd file <filename>\n"
				+ "\tdelete file <filename>\n"
				+ "\tcheck file <filname>\n"
				+ "\tupdate file <filename>\n"
				+ "\tadd files <regexp>";

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

//	logger << "\"" +  file  + "\"";

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

void WorkSpace::SendEmail(const std::string& message, const std::string& mail) {
	std::string query = "echo \"" + message + "\" | mail -s \"file-integrity\" \"" + mail  + "\"";
	logger << query;
	logger << mail;
	system(query.c_str());
}

void* WorkSpace::RecCheck(void*) {
	while (true) {
		for (auto it = checksum_table.begin(); it != checksum_table.end(); ++it) {
			if (!CheckFile(it.Key())) {
				std::string message = "File " + std::string(it.Key()) + " has changed.";
				SendEmail(
					"File " + std::string(it.Key()) + " has changed.",
					"ilyshnikova@yandex.ru"
				);
				logger << std::string("!!!!  ")	+ std::string(it.Key())  + "  changed !!!!";

			} else {
				logger << std::string(it.Key())  + " ok";
			}
			AddCheckSum(std::string(it.Key()));
		}

			sleep(5);
	}
	return NULL;
}

void WorkSpace::Inotify() {
	size_t EVENT_SIZE(sizeof (struct inotify_event));
	size_t EVENT_BUF_LEN(1024 * (EVENT_SIZE + 16));
	char buffer[EVENT_BUF_LEN];

	while (1) {
		int i = 0;

		int length = read(inotify, buffer, EVENT_BUF_LEN);

		if (length < 0) {
			perror("read");
		}

		while (i < length) {
			struct inotify_event *event = (struct inotify_event *) &buffer[i];

			std::string file_name;

			try {
				file_name = std::string(files_by_ind_table.Select(event->wd));
			} catch (std::exception& e) {
				continue;
			}
			if (event->mask & IN_CLOSE_WRITE && !CheckFile(file_name)) {
				logger << "file " + file_name + "was close after opet to write";

				SendEmail(
					"File " + file_name + " has changed.",
					"ilyshnikova@yandex.ru"
				);
				logger << "file " + file_name + " changed(inotify)";
				AddCheckSum(file_name);
			}

			if (event->mask & IN_ATTRIB) {
				std::string file_name = std::string(files_by_ind_table.Select(event->wd));
				SendEmail(
					"Attribs of file " + file_name + " has changed.",
					"ilyshnikova@yandex.ru"
				);
				logger << "attr of file " + file_name + " changed(inotify)";
			}

			i += EVENT_SIZE + event->len;
		}
	}
}

void* WorkSpace::StatInotify(void* arg) {
	((WorkSpace*)arg)->Inotify();
	return NULL;
}

void WorkSpace::AddFileToInotify(const std::string& file_name) {

	pthread_mutex_lock(&m);
	files_by_ind_table.Insert(
		inotify_add_watch(inotify, file_name.c_str(), IN_ALL_EVENTS),
		file_name
	);
	ind_by_files_table.Insert(
		file_name,
		inotify_add_watch(inotify, file_name.c_str(), IN_ALL_EVENTS)
	);
	pthread_mutex_unlock(&m);
}

void WorkSpace::AddFilesToInotify() {
	for (auto it = checksum_table.begin(); it != checksum_table.end(); ++it) {
		logger << "add file " + std::string(it.Key())  + " to inotify";
		AddFileToInotify(it.Key());
	}
}
