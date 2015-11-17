#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include "BerkeleyDB.h"
#include "exception.h"
#include "logger.h"


/*   StringType    */

StringType::StringType()
: value()
{}

std::string StringType::GetValue() const {
	return value;
}


StringType::StringType(const std::string& value)
: value(value)
{}


StringType::StringType(const int value)
: value(std::to_string(value))
{}

StringType::StringType(const double v)
: value(std::to_string(v))
{}

StringType::StringType(const std::time_t value)
: value(std::to_string((long long int)value))
{}

StringType::StringType(char* value)
: value(std::string(value))
{}


StringType::operator std::string() const {
	return value;
}

StringType::operator double() const {
	return std::stod(value);
}



std::vector<std::string> Split(const std::string& string, const char separator) {
	std::vector<std::string> result;
	std::string substr;

	for (size_t i = 0; i < string.size(); ++i) {
		if (string[i] == separator) {
			result.push_back(substr);
			substr = "";
			if (i + 1 == string.size()) {
				result.push_back(substr);
			}
		} else if (i + 1 == string.size()) {
			result.push_back(substr + string[i]);
		} else {
			substr += string[i];
		}
	}

	return result;
}


Table::Table(const std::string& table_name, const std::string& path)
: table_name()
, path()
, env(0)
, pdb()
, select_query()
{
	env.set_error_stream(&logger);
	env.open(path.c_str(), DB_CREATE | DB_INIT_MPOOL, 0);
	pdb = new Db(&env, 0);
	pdb->open(NULL, table_name.c_str(), NULL, DB_BTREE, DB_CREATE, 0);
}


Table::Rows::Rows(Db* pdb)
: cursorp()
, index(0)
, key()
, value()
, is_end(false)
{
	pdb->cursor(NULL, &cursorp, 0);
}

bool Table::Rows::operator==(const Rows& other) const {
	if (!is_end && !other.is_end) {
		return true;
	}
	return index == other.index && is_end == other.is_end;
}

bool Table::Rows::operator!=(const Rows& other) const {
	return !(other == *this);
}

Table::Rows& Table::Rows::operator++() {

	++index;
	int ret = cursorp->get(&key, &value, DB_NEXT);
	is_end = !ret;
	return *this;
}

StringType Table::Rows::Key() const {
	return StringType((char*)key.get_data());
}

StringType Table::Rows::Value() const {
	return StringType((char*)value.get_data());
}


Table& Table::Insert(const StringType& key, const StringType& value) {
	Dbt db_key(const_cast<char*>(std::string(key).data()), std::string(key).size());
	Dbt db_value(const_cast<char*>(std::string(value).data()), std::string(value).size() + 1);

	pdb->put(NULL, &db_key, &db_value, 0);

	return *this;
}


StringType Table::Select(const std::string& key) {
	Dbt db_key(const_cast<char*>(std::string(key).data()), std::string(key).size());
	char buffer[1024];
	Dbt data;
	data.set_data(buffer);
	data.set_ulen(1024);
	data.set_flags(DB_DBT_USERMEM);
	if (pdb->get(NULL, &db_key, &data, 0) == DB_NOTFOUND) {
		throw FIException(145131, "Value with key " + key + " does not exist.");
	}

	return StringType((char*)(data.get_data()));
}

void Table::Delete(const std::string& key) const {
	if (!DoesKeyExist(key))  {
		throw FIException(120312, "Key " + key + " does not exist in db.");
	}
	Dbt dkey(const_cast<char*>(std::string(key).data()), std::string(key).size());
	if (pdb->del(NULL, &dkey, 0) != 0) {
		throw FIException(124532, "Can't delete " + key + ".");
	}
}

bool Table::DoesKeyExist(const std::string& key) const {
	Dbt data;
	Dbt dkey(const_cast<char*>(std::string(key).data()), std::string(key).size());
	return (pdb->get(NULL, &dkey, &data, 0) != DB_NOTFOUND);

}

Table::~Table() {
	if (pdb != NULL) {
		pdb->close(0);
		delete pdb;
	}

	env.close(0);

}
