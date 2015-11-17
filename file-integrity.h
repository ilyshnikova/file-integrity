#ifndef FI
#define FI

#include "BerkeleyDB.h"
#include "daemon.h"

class WorkSpace : public DaemonBase {
private:
	Table checksum_table;

	std::string Respond(const std::string& query);

	bool CheckFile(const std::string& file_name);

	std::string Evaluation(const std::string& string) const;

	std::string GetAllFile(const std::string& file_name) const;

	std::string FilesEvaluation(const std::string& file_name) const;

	void AddCheckSum(const std::string& file_name);

public:

	WorkSpace();
};


#endif
