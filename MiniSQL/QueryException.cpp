#include "QueryException.h"


QueryException::QueryException(const char * message):
	exception(message)
{
}


QueryException::~QueryException()
{
}

