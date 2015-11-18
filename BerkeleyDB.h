#ifndef BDB
#define BDB
#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <stdlib.h>

#include <iostream>
#include <iomanip>

#include <db_cxx.h>
#include "exception.h"
#include "logger.h"



std::vector<std::string> Split(const std::string& string, const char separator);


class StringType {
private:
	std::string value;
public:
	StringType();

	std::string GetValue() const;

	StringType(const std::string& value);

	StringType(const int value);

	StringType(const double value);

	StringType(const std::time_t value);

	StringType(char* value);

	operator std::string() const;

	operator double() const;


};



class Table {
private:
	std::string table_name;
	std::string path;
	DbEnv env;
	Db* pdb;
	std::vector<std::unordered_map<std::string, StringType> > select_query;


	class Rows {
	private:
		Dbc *cursorp;
		size_t index;
		Dbt key;
		Dbt value;
		bool is_end;

	public:

		Rows(Db* pdb, bool is_end, const std::string& table_name, const std::string& path);

		bool operator==(const Rows& other) const;

		bool operator!=(const Rows& other) const;

		Rows& operator++();

		StringType Key() const;

		StringType Value() const;
	};


public:

	typedef Rows rows;

	Table(const std::string& table_name, const std::string& path);

	Table& Insert(const StringType& key, const StringType& value);

	StringType Select(const std::string& key) const;

	void Delete(const std::string& keuy) const;

	bool DoesKeyExist(const std::string& key) const;

	Rows begin();

	Rows end();

	~Table();
};


#endif
