#pragma once

#include <stdexcept>

class QueryException : public std::exception
{
public:
	QueryException(const char * message);
	virtual ~QueryException();
};

