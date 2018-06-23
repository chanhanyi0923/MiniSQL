#pragma once
#include <string>
class Attribute
{
public:
	//data type：-1 int, 0 float, 1~255 char
	int flag[32];

	//attribute name
	std::string name[32];

	//unique  设置主键
	bool unique[32];

	int num;
	Attribute();
	virtual ~Attribute();
};

