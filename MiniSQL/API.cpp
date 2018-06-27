#include "API.h"


#include <iostream>

#include <boost/lexical_cast.hpp>

#include "BufferBlock.h"
#include "IndexManager.h"
#include "CatalogManager.h"
#include "QueryException.h"
#include "Data.h"
#include "DataC.h"
#include "DataF.h"
#include "DataI.h"

API::API()
{
}


API::~API()
{
}


Data * API::toData(const int attr_flag, const string & value)
{
	Data * data = nullptr;
	if (attr_flag == -1) {
		// int
		int d;
		try {
			d = boost::lexical_cast<int>(value);
		}
		catch (...) {
			throw QueryException("the format of length is invalid.");
		}

		data = new DataI(d);
	}
	else if (attr_flag == 0) {
		// float
		float d;
		try {
			d = boost::lexical_cast<float>(value);
		}
		catch (...) {
			throw QueryException("the format of length is invalid.");
		}

		data = new DataF(d);
	}
	else {
		// char
		if (value.front() == '\'' && value.back() == '\'') {
			data = new DataC(value.substr(1, value.size() - 2));
		} else if (value.front() == '"' && value.back() == '"') {
			data = new DataC(value.substr(1, value.size() - 2));
		} else {
			data = new DataC(value);
		}
	}
	return data;
}


void API::toWhere(vector<Where> & where_conds, const vector<Condition> & conds, Table * table_ptr)
{
	for (const auto & cond : conds) {
		Where w;

		std::string op;
		switch (cond.type) {
		case Condition::Equal:
			w.flag = Where::eq;
			break;
		case Condition::NotEqual:
			w.flag = Where::neq;
			break;
		case Condition::Greater:
			w.flag = Where::g;
			break;
		case Condition::Less:
			w.flag = Where::l;
			break;
		case Condition::GreaterOrEqual:
			w.flag = Where::geq;
			break;
		case Condition::LessOrEqual:
			w.flag = Where::leq;
			break;
		}

		const auto & table_attr = table_ptr->getattribute();

		for (int i = 0; i < table_attr.num; i++) {
			const string & attr_name = table_attr.name[i];
			const int attr_flag = table_attr.flag[i];
			if (cond.column == attr_name) {
				w.d = API::toData(attr_flag, cond.value);
			}
		}

		where_conds.push_back(w);
	}
}

void API::select(
	const std::string & table,
	const std::vector<std::string> & columns,
	const std::vector<Condition> & conds
)
{
	CatalogManager catalog_manager;
	RecordManager record_manager;

	Table * table_ptr = catalog_manager.getTable(table);
	//Table * table_ptr = nullptr;

	//Table output = api.Select(*t, attrselect, attrwhere, w);

	//return rm.Select(tableIn, attrSelect, mask, w);

	vector<int> select_indices;

	vector<Where> where_conds;
	vector<int> where_indices;

	API::toWhere(where_conds, conds, table_ptr);

	const auto & table_attr = table_ptr->getattribute();
	
	for (int i = 0; i < table_attr.num; i++) {
		const string & attr_name = table_attr.name[i];
		for (const auto & cond : conds) {
			if (cond.column == attr_name) {
				where_indices.push_back(i);
			}
		}

		for (const auto & column : columns) {
			if (column == attr_name) {
				select_indices.push_back(i);
			}
		}
	}

	if (columns.size() == 1 && columns.back() == "*") {
		for (int i = 0; i < table_attr.num; i++) {
			select_indices.push_back(i);
		}
	}

	Table result = record_manager.Select(*table_ptr, select_indices, where_indices, where_conds);
	result.display();
}


void API::deleteFrom(
	const std::string & table,
	const std::vector<Condition> & conds
)
{
	CatalogManager catalog_manager;
	RecordManager record_manager;

	Table * table_ptr = catalog_manager.getTable(table);

	vector<Where> where_conds;
	vector<int> where_indices;

	API::toWhere(where_conds, conds, table_ptr);

	const auto & table_attr = table_ptr->getattribute();

	for (int i = 0; i < table_attr.num; i++) {
		const string & attr_name = table_attr.name[i];
		for (const auto & cond : conds) {
			if (cond.column == attr_name) {
				where_indices.push_back(i);
			}
		}
	}

	record_manager.Delete(*table_ptr, where_indices, where_conds);

	//std::cout << "----------" << std::endl;
	//std::cout << "Table: {" << table << "}" << std::endl;

	//std::cout << "Where: " << std::endl;
	//for (const auto & cond : conds) {
	//	std::cout << "    {" << cond.column << "} ";

	//	std::string op;
	//	switch (cond.type) {
	//	case Condition::Equal:
	//		op = "=";
	//		break;
	//	case Condition::NotEqual:
	//		op = "<>";
	//		break;
	//	case Condition::Greater:
	//		op = ">";
	//		break;
	//	case Condition::Less:
	//		op = "<";
	//		break;
	//	case Condition::GreaterOrEqual:
	//		op = ">=";
	//		break;
	//	case Condition::LessOrEqual:
	//		op = "<=";
	//		break;
	//	}

	//	std::cout << op << " {" << cond.value << "}" << std::endl;
	//}
	//std::cout << "----------" << std::endl;
}


void API::insert(
	const std::string & table,
	const std::vector<std::string> & values
)
{
	CatalogManager catalog_manager;
	RecordManager record_manager;

	Table * table_ptr = catalog_manager.getTable(table);

	const auto & table_attr = table_ptr->getattribute();

	Tuple tuple;
	for (size_t i = 0; i < values.size(); i++) {
		const std::string & value = values[i];
		const int attr_flag = table_attr.flag[i];
		Data * data = API::toData(attr_flag, value);
		tuple.addData(data);
	}

	record_manager.Insert(*table_ptr, tuple);
}


void API::createTable(
	const std::string & table,
	const std::string & primary_key,
	const std::vector<ColumnAttribute> & attrs
)
{
	Index index;
	index.num = 0;

	short primary = -1;
	Attribute atb;
	atb.num = attrs.size();
	for (int i = 0; i < attrs.size(); i++) {
		const ColumnAttribute & attr = attrs[i];

		int &type = atb.flag[i];
		atb.name[i] = attr.name;
		atb.unique[i] = attr.unique;

		switch (attr.type) {
		case ColumnAttribute::Character:
			type = attr.length;
			break;
		case ColumnAttribute::Integer:
			type = -1;
			break;
		case ColumnAttribute::Float:
			type = 0;
			break;
		}

		if (attr.name == primary_key) {
			primary = i;
			atb.unique[i] = true;
		}
	}

	BufferBlock buffer;
	CatalogManager catalog_manager;
	RecordManager record_manager;

	catalog_manager.create_table(table, atb, primary, index);

	Table * table_ptr = catalog_manager.getTable(table);
	record_manager.CreateTable(*table_ptr, buffer);

	if (primary != -1) {
		record_manager.CreateIndex(*table_ptr, primary);
	}

	delete table_ptr;
}

void API::dropTable(
	const std::string & table
)
{

	//Table* t = cm.getTable(temp);

	//if (t->index.num>0) {
	//	for (int j = 0; j<t->index.num; j++) {
	//		cm.drop_index(temp, t->index.indexname[j]);
	//	}
	//}

	//if (qs[pos1 + 1] != ';')
	//{
	//	delete t;
	//	throw QueryException("ERROR: invalid query format!");
	//}


	//delete  t;
	//cm.drop_table(temp);

	//std::cout << "----------" << std::endl;
	//std::cout << "Table: {" << table << "}" << std::endl;

	//void CataManager::drop_table(std::string t) {
	//	if (!hasTable(t))
	//		throw TableException("ERROR in drop_table: No table named " + t);
	//	Table* tb = getTable(t);
	//	API api;
	//	api.DropTable(*tb);
	//	remove(("T_" + t).c_str());
	//}

	//bool res;
	//res = rm.DropTable(tableIn);
	//for (int i = 0; i < tableIn.index.num; i++) {
	//	DropIndex(tableIn, tableIn.index.location[i]);
	//}
	//return res;
}

void API::createIndex(
	const std::string & table,
	const std::string & column,
	const std::string & index
)
{
	CatalogManager catalog_manager;
	RecordManager record_manager;

	Table * table_ptr = catalog_manager.getTable(table);

	const auto & table_attr = table_ptr->getattribute();

	int column_index = -1;

	for (int i = 0; i < table_attr.num; i++) {
		if (table_attr.name[i] == column) {
			column_index = i;
			break;
		}
	}

	if (column_index == -1) {
		throw QueryException(("Column " + column + " not found.").c_str());
	}


	record_manager.CreateIndex(*table_ptr, column_index);

	//const string filename = table + "_" + column + ".index";

	//// 0 for int, 1 for float, 2 for char
	//int key_type;
	//switch (table_attr.flag[column_index]) {
	//case -1: // int
	//	key_type = 0;
	//	break;
	//case 0: // float
	//	key_type = 1;
	//	break;
	//default: // char
	//	key_type = 2;
	//	break;
	//}

	//index_manager.Init(filename, key_type);

	//Table result = record_manager.Select(*table_ptr, select_indices);
	//for (int i = 0; i < result.T.size(); i++) {
	//	index_manager.Insert(filename, result.T[i]->data[column_index], i);
	//}

	//std::cout << "----------" << std::endl;
	//std::cout << "Table: {" << table << "}" << std::endl;
	//std::cout << "Column: {" << column << "}" << std::endl;
	//std::cout << "Index: {" << index << "}" << std::endl;
}

void API::dropIndex(
	const std::string & index
)
{

	//Table* temp = getTable(tname);
	//try {
	//	temp->dropindex(iname);
	//	drop_table(tname);
	//	create_table(tname, temp->attr, temp->primary, temp->index);
	//	delete temp;
	//}
	//catch (TableException e1) {
	//	delete temp;
	//	throw e1;
	//}


	std::cout << "----------" << std::endl;
	std::cout << "Index: {" << index << "}" << std::endl;
}

