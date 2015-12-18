#ifndef FI
#define FI

#include "BerkeleyDB.h"
#include "daemon.h"

class WorkSpace : public DaemonBase {
private:
	Table checksum_table;
	Table files_by_ind_table;
	Table ind_by_files_table;
	int inotify;
	pthread_mutex_t m;



	std::string Respond(const std::string& query);

	bool CheckFile(const std::string& file_name) const;

	std::string Evaluation(const std::string& string) const;

	std::string GetAllFile(const std::string& file_name) const;

	std::string FilesEvaluation(const std::string& file_name) const;

	void AddCheckSum(const std::string& file_name);

	void* RecCheck(void*);

	void Inotify();

	static void* StatInotify(void*);

	void SendEmail(const std::string& message, const std::string& mail);

	void AddFileToInotify(const std::string& file_name);

	void AddFilesToInotify();

public:

	WorkSpace();
};


#endif
