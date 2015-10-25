#ifndef EXCEPTION
#define EXCEPTION

#include <exception>
#include <string>

class FIException : public std::exception {
private:
	int id;
	std::string information;
public:
	FIException(const int id, const std::string& information);

	const char * what() const throw();

	~FIException() throw();
};

#endif
