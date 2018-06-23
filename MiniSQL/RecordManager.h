#pragma once

#include <string>
#include "BufferBlock.h"
#include "Where.h"
#include "Table.h"

#define BLOCK_SIZE 4096
#define MAXBLOCKNUM 100
#define EMPTY  '#'
#define DELETED '#'
#define NOTEMPTY 1
#define MAXSTRINGLEN 100
#define MIN_Theta 0.0001

using std::string;
using std::vector;

class RecordManager
{
public:
	RecordManager();
	RecordManager(BufferBlock bf);
	virtual ~RecordManager();

	bool CreateTable(Table& tableIn, BufferBlock buffer);

	Tuple* Char2Tuple(Table& tableIn, char* stringRow);
	char* Tuple2Char(Table& tableIn, Tuple& singleTuper);
	Tuple String2Tuple(Table& tableIn, string stringRow);

	//这个用index来做一次比较快速的查找
	//int FindWithIndex(Table& tableIn, tuper& row, int mask);
	//bool CreateIndex(Table& tableIn, int attr);
	//void InsertWithIndex(Table& tableIn, tuper& singleTuper);
	void Insert(Table& tableIn, Tuple& singleTuple);
	int Delete(Table& tableIn, vector<int>mask, vector<Where> w);
	Table SelectProject(Table& tableIn, vector<int>attrSelect);
	bool isSatisfied(Table& tableinfor, Tuple& row, vector<int> mask, vector<Where> w);
	Table Select(Table& tableIn, vector<int>attrSelect, vector<int>mask, vector<Where>& w);
	Table Select(Table& tableIn, vector<int>attrSelect);
	bool DropTable(Table& tableIn);

	int addBlockInFile(Table& tableinfor);
	bool UNIQUE(Table& tableinfo, Where w, int loca);
	//private:

	BufferBlock buffer;
};

