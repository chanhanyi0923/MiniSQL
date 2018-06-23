#pragma once

#include <string>

class Condition
{
public:
	enum Type { Equal, LessOrEqual, Less, GreaterOrEqual, Greater, NotEqual };

	Type type;
	std::string column, value;

	Condition();
	virtual ~Condition();
};

