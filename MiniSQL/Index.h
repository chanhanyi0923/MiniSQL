#pragma once

#include <string>

class Index
{
public:
	//index个数
	int num;

	//存的值是attribute的位置
	short location[10];

	//index名字
	std::string indexname[10];
	Index();
	virtual ~Index();
};

