#include "DataC.h"



DataC::DataC(std::string data)
{
	flag = data.length();
	if (flag == 0) {
		flag = 1;
	}
}


DataC::~DataC()
{
}


