#pragma once

#include <string>
#include <vector>
#include "Attribute.h"
#include "Tuple.h"
#include "Index.h"

class Table
{
public:
	Table();
	virtual ~Table();

	friend class CataManager;
	//Construct with Tname and column.
	Table(std::string s, Attribute aa, int bn);
	Table(const Table& t);

	int blockNum; //total number of blocks occupied in data file;
	std::string Tname;
	Attribute attr;//number of attributes
	std::vector<Tuple*> T;//pointers to each tuple
	int primary;//the location of primary key（就是第几个attribute）. -1 means no primary key.
	Index index;
	int linklist[1000];

	//一个tuple的大小，也是一个record的长度
	int dataSize();//size of all attribute

	Attribute getattribute();

	void setindex(short i, std::string iname);
	void dropindex(std::string iname);
	void Copyindex(Index ind);

	Index Getindex();

	//set the primary key
	void setprimary(int p);

	void display();

	std::string getname();

	int getattNum() const;

	int getRsize() const;

	void addData(Tuple* t);
};

