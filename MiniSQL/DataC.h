#pragma once
#include "Data.h"
#include <string>
class DataC :
	public Data
{
public:
	std::string x;
	DataC(std::string data);
	virtual ~DataC();
};

