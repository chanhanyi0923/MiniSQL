#pragma once
#include "Data.h"
class Where
{
public:
	Where();
	~Where();

	enum Type { eq, leq, l, geq, g, neq };

	Data* d;
	Type flag;
};

