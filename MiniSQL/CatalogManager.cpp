#include "CatalogManager.h"

#ifdef DEBUG
#include <iostream>
#endif

#include <fstream>
#include <string>


CatalogManager::CatalogManager()
{
}

CatalogManager::~CatalogManager()
{
}

using namespace std; 
//给所有table项的文件名字前加上Table，所有index名字前加上Index
bool CatalogManager::hasTable(std::string name) {
	name = "Table" + name;
	ifstream in(name);
//cout << "finding " << endl;
	if (!in) return false;
	else {
		in.close();
		return true;
	}
}

void CatalogManager::create_table(string name, Attribute atb, int primary, Index index) {
	if (hasTable(name))
		throw ("ERROR in mycatalogmanager::create_table: table: " + name + "exist");
	if (primary >= 0) atb.unique[primary] = true;
	//创建文件
	string filename = "Table" + name;
	fstream fout(filename, ios::out);
	fout.close();
	//获取指定block在BufferBlock中的序号
#ifdef DEBUG
	int blockNum = buffer.write_block(filename, 1);//    1表示table/0表示index
#else
	int blockNum = 0;
#endif

	//通过序号在m_blocks[MAX_BLOCKS]中找到对应的内存信息（address）
	//先写入一些文件头信息,attribute个数，index个数，以及占几个block，主键primary是谁
	char* begin = BufferBlock::m_blocks[blockNum].address;
	int pos = 0;
	memcpy(&begin[pos], &atb.num, sizeof(int));
	//int a = *(int*)(begin+pos);
	pos = pos + sizeof(int);
	memcpy(&begin[pos], &index.num, sizeof(int));
	//int b = *(int*)(begin + pos);
	pos = pos + sizeof(int);
	int bn = 0;
	memcpy(&begin[pos], &bn, sizeof(int));
	//int c = *(int*)(begin + pos);
	pos = pos + sizeof(int);
	memcpy(&begin[pos], &primary, sizeof(int));
	//int d = *(int*)(begin + pos);
	pos = pos + sizeof(int);

	//然后写入具体的内存信息
	for (int i = 0; i < atb.num; i++) {//将attribute的name写入
		memcpy(&begin[pos], atb.name[i].data(), atb.name[i].length() * sizeof(char));
		pos = pos + (int)atb.name[i].length() * sizeof(char);
		memcpy(&begin[pos], "\0", sizeof(char));
		pos += sizeof(char);
	}

	for (int i = 0; i < atb.num; i++) {//记录每个attribute的datatype
		memcpy(&begin[pos], &atb.flag[i], sizeof(int));
		pos = pos + sizeof(int);
	}

	for (int i = 0; i < atb.num; i++) {//记录unique的信息
		memcpy(&begin[pos], &atb.unique[i], sizeof(bool));
		pos = pos + sizeof(bool);
	}

	for (int i = 0; i < index.num; i++) {//记录indexname，index.num为index个数
		memcpy(&begin[pos], index.indexname[i].data(), index.indexname[i].length() * sizeof(char));
		pos = pos + (int)index.indexname[i].length() * sizeof(char);
		memcpy(&begin[pos], "\0", sizeof(char));
		pos += sizeof(char);
	}

	for (int i = 0; i < index.num; i++) {//记录index.location
		memcpy(&begin[pos], &index.location[i], sizeof(short));
		pos = pos + sizeof(short);
	}
	BufferBlock::m_blocks[blockNum].not_being_used();
	BufferBlock::m_blocks[blockNum].written();//给这个位置标记：已经写过
}

void CatalogManager::recreate_table(string name, Attribute atb, int primary, Index index) {
	if (hasTable(name))
		throw ("ERROR in mycatalogmanager::create_table: table: " + name + "exist");
	if (primary >= 0) atb.unique[primary] = true;
	//创建文件
	string filename = "Table" + name;
	fstream fout(filename, ios::out);
	fout.close();
	//获取指定block在BufferBlock中的序号
	int blockNum = buffer.read_block(filename, 0, 1);   //1表示table/2表示index
	//通过序号在m_blocks[MAX_BLOCKS]中找到对应的内存信息（address）
	//先写入一些文件头信息,attribute个数，index个数，以及占几个block，主键primary是谁
	char* begin = BufferBlock::m_blocks[blockNum].address;
	int pos = 0;
	memcpy(&begin[pos], &atb.num, sizeof(int));
	pos = pos + sizeof(int);
	memcpy(&begin[pos], &index.num, sizeof(int));
	pos = pos + sizeof(int);
	int bn = 0;//
	memcpy(&begin[pos], &bn, sizeof(int));
	pos = pos + sizeof(int);
	memcpy(&begin[pos], &primary, sizeof(int));
	pos = pos + sizeof(int);

	//然后写入具体的内存信息
	for (int i = 0; i < atb.num; i++) {//将attribute的name写入
		memcpy(&begin[pos], atb.name[i].data(), atb.name[i].length() * sizeof(char));
		pos = pos + (int)atb.name[i].length() * sizeof(char);
		memcpy(&begin[pos], "\0", sizeof(char));
		pos += sizeof(char);
	}

	for (int i = 0; i < atb.num; i++) {//记录每个attribute的datatype
		memcpy(&begin[pos], &atb.flag[i], sizeof(int));
		pos = pos + sizeof(int);
	}

	for (int i = 0; i < atb.num; i++) {//记录unique的信息
		memcpy(&begin[pos], &atb.unique[i], sizeof(bool));
		pos = pos + sizeof(bool);
	}

	for (int i = 0; i < index.num; i++) {//记录indexname，index.num为index个数
		memcpy(&begin[pos], index.indexname[i].data(), index.indexname[i].length() * sizeof(char));
		pos = pos + (int)index.indexname[i].length() * sizeof(char);
		memcpy(&begin[pos], "\0", sizeof(char));
		pos += sizeof(char);
	}

	for (int i = 0; i < index.num; i++) {//记录index.location
		memcpy(&begin[pos], &index.location[i], sizeof(short));
		pos = pos + sizeof(short);
	}
	BufferBlock::m_blocks[blockNum].not_being_used();
	BufferBlock::m_blocks[blockNum].written();//给这个位置标记：已经写过
}

Table* CatalogManager::getTable(string name) {
	if (!hasTable(name))
		throw ("ERROR in getTable: No table named " + name);
	Attribute atb;
	Index ind;
	int pri;//primary
	int bn;//blocknumber
	name = "Table" + name;
		   //获取指定block在内存中的序号**********************************
	int blockNum  = buffer.read_block(name,0,1);
	//通过序号在m_blocks[MAX_BLOCKS]中找到对应的内存信息（address）
	char* begin = BufferBlock::m_blocks[blockNum].address;
	int pos = 0;
	//读表头
	memcpy(&atb.num, &begin[pos], sizeof(int));
	int a = atb.num;
	pos = pos + sizeof(int);
	memcpy(&ind.num, &begin[pos], sizeof(int));
	int b = ind.num;
	pos = pos + sizeof(int);
	memcpy(&bn, &begin[pos], sizeof(int));
	int c = bn;
	pos = pos + sizeof(int);
	memcpy(&pri, &begin[pos], sizeof(int));
	int d = pri;
	pos = pos + sizeof(int);
	int pos1;

	char temp[20];//暂存attribute的名字
	int i;
	for (i = 0; i < atb.num; i++) {
		pos1 = 0;
		while (begin[pos+pos1] != '\0')
			pos1++;
		memcpy(temp, &begin[pos], (++pos1) * sizeof(char));
		atb.name[i] = temp;//string可以直接用 = 赋值

		pos = pos + (pos1) * sizeof(char);
	}

	for (i = 0; i < atb.num; i++) {
		memcpy(&atb.flag[i], &begin[pos], sizeof(int));
		pos = pos + sizeof(int);
	}

	for (i = 0; i < atb.num; i++) {
		memcpy(&atb.unique[i], &begin[pos], sizeof(bool));
		pos = pos + sizeof(bool);
	}

	for (i = 0; i < ind.num; i++) {
		pos1 = 0;
		while (begin[pos+pos1] != '\0')
			pos1 += 1;
		memcpy(temp, &begin[pos], (pos1+1) * sizeof(char));
		ind.indexname[i] = temp;
		pos = pos + (pos1) * sizeof(char);
	}

	for (i = 0; i < ind.num; i++) {
		memcpy(&ind.location[i], &begin[pos], sizeof(short));
		pos = pos + sizeof(short);
	}

	//bf.using_block(blockNum);//用了一次

	Table* t = new Table(name, atb, bn);
	t->Copyindex(ind);
	t->setprimary(pri);
	return t;

}

void CatalogManager::create_index(string tname, string aname, string iname) {
	Table* temp = getTable(tname);
	try {
		int i;
		for (i = 0; i < temp->getattNum(); i++)
			if (temp->attr.name[i] == aname)
				break;//叫这个名字的属性在第i个
		if (i == temp->getattNum())//没有找到叫这个名字的属性
			throw ("No attribute named " + aname);
		if (temp->attr.unique[i] == 0)//不具有唯一性的属性不能作index
			throw ("This attribute is not unique!");
		temp->setindex(i, iname);
//	cout << "index numaaaaa" << temp->index.num << endl;
		int blockNum = buffer.read_block(tname, 0, 1);//1 for table
		char* begin = BufferBlock::m_blocks[blockNum].address;

		int pos = 0;
		Attribute atb = temp->attr;
		Index index = temp->index;
		int primary = temp->primary;
		//cout << "primary::" <<primary << endl;


		memcpy(&begin[pos], &atb.num, sizeof(int));
		pos = pos + sizeof(int);
		memcpy(&begin[pos], &index.num, sizeof(int));
		pos = pos + sizeof(int);
		int bn = 0;
		memcpy(&begin[pos], &bn, sizeof(int));
		pos = pos + sizeof(int);
		memcpy(&begin[pos], &primary, sizeof(int));
		pos = pos + sizeof(int);


		for (i = 0; i < atb.num; i++) {
			memcpy(&begin[pos], atb.name[i].data(), atb.name[i].length() * sizeof(char));
			pos = pos + (int)atb.name[i].length() * sizeof(char);
			memcpy(&begin[pos], "\0", sizeof(char));
			pos += sizeof(char);
		}

		for (i = 0; i < atb.num; i++) {
			memcpy(&begin[pos], &atb.flag[i], sizeof(int));
			pos = pos + sizeof(int);
		}

		for (i = 0; i < atb.num; i++) {
			memcpy(&begin[pos], &atb.unique[i], sizeof(bool));
			pos = pos + sizeof(bool);
		}

		for (i = 0; i < index.num; i++) {
			memcpy(&begin[pos], index.indexname[i].data(), index.indexname[i].length() * sizeof(char));
			pos = pos + (int)index.indexname[i].length() * sizeof(char);
			memcpy(&begin[pos], "\0", sizeof(char));
			pos += sizeof(char);
		}

		for (i = 0; i < index.num; i++) {
			memcpy(&begin[pos], &index.location[i], sizeof(short));
			pos = pos + sizeof(short);
		}

		//buffer.using_block(blockNum);
		delete temp;
	}
	catch (string event) {
		delete temp;
		throw event;
	}
}

void CatalogManager::drop_table(string t) {
	//t = "Table" + t;
	if (!hasTable(t))
		throw ("ERROR in drop_table: No table named " + t);
	Table* tb = getTable(t);
	/*API api;
	api.DropTable(*tb);*/
	//删除同名的文件
	remove(("Table" + t).c_str());
	//还要删除储存这些具体数据的文件,需要调用record里函数

}

void CatalogManager::drop_index(string tname, string iname) {
	Table* temp = getTable(tname);
	try {
		temp->dropindex(iname);
		drop_table(tname);
		recreate_table(tname, temp->attr, temp->primary, temp->index);
		delete temp;
	}
	catch (string event) {
		delete temp;
		throw event;
	}
}

void CatalogManager::show_table(string tname) {
	Table* t = getTable(tname);
	cout << tname << ":" << endl;
	Index ind;
	ind = t->Getindex();
//cout << "index numaaaaa" << ind.num << endl;
	cout << t->getattNum() << endl;
	for (int i = 0; i < t->getattNum(); i++) {
		cout << t->attr.name[i] << " ";
		if (t->attr.flag[i] == -1)
			cout << "int" << " ";
		else if (t->attr.flag[i] == 0)
			cout << "float" << " ";
		else cout << "char(" << t->attr.flag[i] << ") ";
		if (t->attr.unique[i] == 1)
			cout << "unique ";
		if (i == t->primary)
			cout << "primary key";
		cout << endl;
	}
	if (ind.num > 0) {
		cout << "index: ";
		for (int i = 0; i < ind.num; i++) {
			cout << ind.indexname[i] << "(" << t->attr.name[ind.location[i]] << ") ";
		}
		cout << endl;
	}
	delete t;
}

void CatalogManager::changeblock(string tname, int bn) {
	string s = "Table" + tname;
	int blockNum = buffer.read_block(tname, 0, 1);//1 for table，将table头的信息读进来
	char* begin = BufferBlock::m_blocks[blockNum].address;
	int pos = 0;
	pos = pos + sizeof(int);
	pos = pos + sizeof(int);
	memcpy(&begin[pos], &bn, sizeof(int));
	BufferBlock::m_blocks[blockNum].not_being_used();
	BufferBlock::m_blocks[blockNum].written();//给这个位置标记：已经写过
	//bf.using_block(blockNum);
	return;
}
