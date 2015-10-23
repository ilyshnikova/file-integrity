#include "exception.h"

FIException::FIException(const int id, const std::string& information)
: std::exception()
, id(id)
, information(std::string("ERROR ") + std::to_string(id) + std::string(" : ") + information)
{}

const char * FIException::what() const throw() {
	return information.c_str();
}

FIException::~FIException() throw() {}
