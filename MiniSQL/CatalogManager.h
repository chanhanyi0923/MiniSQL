#pragma once

#include "Table.h"
#include "BufferBlock.h"
extern BufferBlock buffer;

using std::string;

class CatalogManager
{
public:
	CatalogManager();
	virtual ~CatalogManager();

	void create_table(string name, Attribute atb, int primary, Index index);
	void recreate_table(string name, Attribute atb, int primary, Index index);
	bool hasTable(string name);
	Table* getTable(string name);
	void create_index(string tname, string aname, string iname);
	void drop_table(string t);
	void drop_index(string tname, string iname);
	void show_table(string tname);
	void changeblock(string tname, int bn);
};
