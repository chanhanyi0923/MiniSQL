#pragma once
#include "Data.h"
class DataI :
	public Data
{
public:
	int x;
	DataI(int data);
	virtual ~DataI();
};

