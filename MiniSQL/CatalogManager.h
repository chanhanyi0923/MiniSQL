#pragma once

#include "Table.h"
#include "BufferBlock.h"

using std::string;

class CatalogManager
{
public:
	CatalogManager();
	virtual ~CatalogManager();

	void create_table(string name, Attribute atb, short primary, Index index, BufferBlock bf);
	void recreate_table(string name, Attribute atb, short primary, Index index, BufferBlock bf);
	bool hasTable(string name);
	Table* getTable(string name, BufferBlock bf);
	void create_index(string tname, string aname, string iname, BufferBlock bf);
	void drop_table(string t, BufferBlock bf);
	void drop_index(string tname, string iname, BufferBlock bf);
	void show_table(string tname, BufferBlock bf);
	void changeblock(string tname, int bn, BufferBlock bf);
};
