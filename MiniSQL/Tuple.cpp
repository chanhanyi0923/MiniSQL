#include "Tuple.h"
#include "Data.h"
#include "DataI.h"
#include "DataF.h"
#include "DataC.h"

#ifdef DEBUG
#include <iostream>
#endif

Tuple::Tuple()
{
}


int Tuple::length() const
{
	return (int)data.size();
}

void Tuple::addData(Data* d)
{
	data.push_back(d);
}

Tuple::Tuple(const Tuple& t)
{
	int i;
	Data* d;
	for (i = 0; i < t.length(); i++) {
		if (t.data[i]->flag == -1) {
			d = new DataI(((DataI*)(t.data[i]))->x);
			this->addData(d);
		}
		if (t.data[i]->flag == 0) {
			d = new DataF(((DataF*)(t.data[i]))->x);
			this->addData(d);
		}
		if (t.data[i]->flag > 0) {
			d = new DataC(((DataC*)(t.data[i]))->x);
			this->addData(d);
		}
	}
}

Tuple::~Tuple()
{
	for (size_t i = 0; i < data.size(); i++)
	{
		if (data[i] == NULL)
			continue;
		if (data[i]->flag == -1)
			delete (DataI*)data[i];
		else if (data[i]->flag == 0)
			delete (DataF*)data[i];
		else
			delete (DataC*)data[i];
	}
}

void Tuple::disptuple() {
	using namespace std;

	for (size_t i = 0; i < data.size(); i++)
	{
		if (data[i] == NULL)
			cout << "NULL" << "\t";
		else if (data[i]->flag == -1)
			cout << ((DataI*)data[i])->x << "\t";
		else if (data[i]->flag == 0)
			cout << ((DataF*)data[i])->x << "\t";
		else cout << ((DataC*)data[i])->x << "\t";
	}
	cout << endl;
}

Data* Tuple::operator[](unsigned short i) {
	if (i >= data.size())
		throw ("out of range in t[i]");
	return data[i];
}


