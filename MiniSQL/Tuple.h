#pragma once
#include <vector>
#include "Data.h"
class Tuple
{
public:
	std::vector<Data*> data;
	Tuple();
	Tuple(const Tuple& t);
	virtual ~Tuple();

	//return the length of the data.
	int length() const;

	//add a new data to the tuple.
	void addData(Data* d);

	//return the pointer to a specified data item.
	Data* operator[](unsigned short i);

	//display the data in the tuple.  
	void disptuple();
};

