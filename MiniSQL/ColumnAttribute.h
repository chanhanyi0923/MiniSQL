#pragma once

#include <string>

class ColumnAttribute
{
public:
	static const int DEFAULT_LENGTH = 4;
	enum Type { Integer, Float, Character };
	Type type;
	int length;
	std::string name;
	bool unique;

	ColumnAttribute();
	virtual ~ColumnAttribute();
};

