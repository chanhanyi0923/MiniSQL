#include "Table.h"

#ifdef DEBUG
#include <iostream>
#endif

Table::Table()
{
}



Table::Table(const Table& t)
{
	this->attr = t.attr;
	this->Tname = t.Tname;
	this->primary = t.primary;
	this->index = t.index;
	this->blockNum = t.blockNum;
	int i;
	for (i = 0; i < t.getRsize(); i++) {
		Tuple* tp = new Tuple(*(t.T[i]));
		this->addData(tp);
	}
}


Table::~Table()
{
	int i;
	int r_size = this->getRsize();
	for (i = 0; i < r_size; i++)
		delete T[i];
}


void Table::addData(Tuple* t)
{
	if (t->length() != attr.num)//插入的data的个数应该等于attribute的个数
		throw ("Illegal Tuper Inserted: unequal column size!");
	//影响效率的话可以删掉这段
	for (int i = 0; i < getattNum(); i++) {
		int j = (*t)[i]->flag;
		if (j > attr.flag[i])
			throw ("Illegal Tuper Inserted: unequal attributes!");
		else if (j < attr.flag[i] && j <= 0)//j是int或float而实际应该为char
			throw ("Illegal Tuper Inserted: unequal attributes!");
	}
	T.push_back(t);
}


void Table::display()
{
	using namespace std;

	for (int i = 0; i < attr.num; i++)
		cout << attr.name[i] << "\t";
	cout << endl;
	for (size_t i = 0; i < T.size(); i++)//tuple.size即为attr.num
		T[i]->disptuple();
	//display the whole table
}


void Table::setindex(short loc, std::string iname) {
	short j;
	for (j = 0; j < index.num; j++)//找一找是否index数组里已经存在了这个index
		if (loc == index.location[j])
			break;
	if (j < index.num)
		throw ("Index already exists on this attribute!");

	for (j = 0; j < index.num; j++)
		if (iname == index.indexname[j])
			break;
	if (j < index.num)
		throw ("Index name has been used!");

	index.location[index.num] = loc;
	index.indexname[index.num] = iname;
	index.num++;
	//cout << "index.num" << index.num << endl;
}

void Table::dropindex(std::string iname) {
	short j;
	for (j = 0; j < index.num; j++)
		if (iname == index.indexname[j])
			break;
	if (j == index.num)
		throw ("No index named " + iname + " on " + Tname + "!");

	std::string temps;
	short tempi;
	temps = index.indexname[j];
	index.indexname[j] = index.indexname[index.num - 1];
	index.indexname[index.num - 1] = temps;

	tempi = index.location[j];
	index.location[j] = index.location[index.num - 1];
	index.location[index.num - 1] = tempi;

	index.num--;

}

int Table::dataSize() {           //size of all attribute
	int res = 0;
	for (int i = 0; i < attr.num; i++) {
		switch (attr.flag[i]) {
		case -1:res += sizeof(int); break;
		case 0:res += sizeof(float); break;
		default:res += attr.flag[i] + 1; break; //多一位储存'\0'
		}
	}
	return res;
}


void Table::Copyindex(Index ind)
{
	index = ind;
}

Index Table::Getindex()
{
	return index;
}

//set the primary key
void Table::setprimary(int p)
{
	primary = p;
}

void display();

std::string Table::getname()
{
	return Tname;
};

//attribute size
int Table::getattNum() const
{
	return attr.num;
};

//record number:tuple number
int Table::getRsize() const
{
	return (int)T.size();
};

Attribute Table::getattribute()
{
	return attr;
}

Table::Table(std::string s, Attribute aa, int bn) :Tname(s), attr(aa), blockNum(bn) {
	primary = -1;
	for (int i = 0; i < 32; i++) { aa.unique[i] = false; }
	index.num = 0;
}
