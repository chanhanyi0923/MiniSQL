#pragma once

#include <string>
#include "BufferBlock.h"
#include "Data.h"

#define MAX_STRING_LENGTH 100
#define blockHead 17
#define INTERNAL 'i'
#define LEAF 'l'
#define ROOTFATHER 0
#define MAX_RANGE 1000
#define BLOCK_SIZE 4096

using std::string;

class IndexManager
{
public:
	IndexManager();
	~IndexManager();


	//初始化结构体。从buffer中读必要信息
	void Init(string file, int keyType);
	
	void Insert(string file, Data* key, int Addr);

	//不存在该元素返回-1，成功删除返回1;
	int Delete(string file, Data* key);

	//返回该key对应的addr
	int Find(string file, Data* key);
	void Drop(string file);

	//闭区间
	int*Range(string file, Data*key1, Data*key2);

	void WriteBack();

private:
	int size;
	int Ktype;
	int Klen;
	string fname;
	unsigned int rootOffset;

	//返回找到的叶节点在文件中的offset
	unsigned int findLeaf(Data* key);


	//找到插入位置，返回pos（offset）,若存在相等key，返回相等位置。若块中最后一个元素也小于key，返回-pos，abs(pos)-gap
	//是最后一个元素。如果比第一个元素还小，返回0
	int find_pos_in_leafBlock(Data* key, unsigned int leafBlockOffset);

	//返回父亲节点的offset
	unsigned int blockSplit(unsigned int splitedBlockOffset);

	void insertRecursively(unsigned int offsetOfRootBlock, Data* key, int addr);
};

