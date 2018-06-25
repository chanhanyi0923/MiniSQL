#include "API.h"


#include <iostream>

#include <boost/lexical_cast.hpp>

#include "QueryException.h"
#include "BufferBlock.h"
#include "IndexManager.h"
#include "CatalogManager.h"

#include "DataC.h"
#include "DataI.h"
#include "DataF.h"


API::API()
{
}


API::~API()
{
}


Data * toData(const int attr_flag, const string & value)
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
	} else if (attr_flag == 0) {
		// float
		float d;
		try {
			d = boost::lexical_cast<float>(value);
		}
		catch (...) {
			throw QueryException("the format of length is invalid.");
		}

		data = new DataF(d);
	} else {
		// char
		data = new DataC(value);
	}
	return data;
}


void API::select(
	const std::string & table,
	const std::vector<std::string> & columns,
	const std::vector<Condition> & conds
)
{
	CatalogManager catalog_manager;
	RecordManager record_manager;

	//Table * table_ptr = catalog_manager.getTable(table);
	Table * table_ptr = nullptr;

	//Table output = api.Select(*t, attrselect, attrwhere, w);

	//return rm.Select(tableIn, attrSelect, mask, w);

	vector<Where> where_conds;
	vector<int> where_indices;
	vector<int> select_indices;

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
				w.d = ::toData(attr_flag, cond.value);
			}
		}

		where_conds.push_back(w);
	}

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

	record_manager.Select(*table_ptr, select_indices, where_indices, where_conds);
	//Table Select(Table& tableIn, vector<int>attrSelect);


	//std::cout << "----------" << std::endl;
	//std::cout << "Table: {" << table << "}" << std::endl;
	//std::cout << "Columns: " << std::endl;

	//if (columns.size() == 1 && columns.front() == "*") {
	//	std::cout << "All Columns." << std::endl;
	//} else {
	//	for (const auto & col : columns) {
	//		std::cout << "    {" << col << "}" << std::endl;
	//	}
	//}

	//std::cout << "Where: " << std::endl;
	//for (const auto & cond : conds) {
	//	std::cout << "    {" << cond.column << "} ";

	//	std::string op;
	//	switch (cond.type) {
	//		case Condition::Equal:
	//			op = "=";
	//			break;
	//		case Condition::NotEqual:
	//			op = "<>";
	//			break;
	//		case Condition::Greater:
	//			op = ">";
	//			break;
	//		case Condition::Less:
	//			op = "<";
	//			break;
	//		case Condition::GreaterOrEqual:
	//			op = ">=";
	//			break;
	//		case Condition::LessOrEqual:
	//			op = "<=";
	//			break;
	//	}

	//	std::cout << op << " {" << cond.value << "}" << std::endl;
	//}
	//std::cout << "----------" << std::endl;
}


void API::deleteFrom(
	const std::string & table,
	const std::vector<Condition> & conds
)
{
	std::cout << "----------" << std::endl;
	std::cout << "Table: {" << table << "}" << std::endl;

	std::cout << "Where: " << std::endl;
	for (const auto & cond : conds) {
		std::cout << "    {" << cond.column << "} ";

		std::string op;
		switch (cond.type) {
		case Condition::Equal:
			op = "=";
			break;
		case Condition::NotEqual:
			op = "<>";
			break;
		case Condition::Greater:
			op = ">";
			break;
		case Condition::Less:
			op = "<";
			break;
		case Condition::GreaterOrEqual:
			op = ">=";
			break;
		case Condition::LessOrEqual:
			op = "<=";
			break;
		}

		std::cout << op << " {" << cond.value << "}" << std::endl;
	}
	std::cout << "----------" << std::endl;
}


void API::insert(
	const std::string & table,
	const std::vector<std::string> & values
)
{
	//CatalogManager catalog_manager;
	//RecordManager record_manager;

	////Table * table_ptr = catalog_manager.getTable(table);
	//Table * table_ptr = nullptr;
	//const auto & table_attr = table_ptr->getattribute();

	//Tuple * tuple = new Tuple;

	//for (size_t i = 0; i < values.size(); i ++) {
	//	const std::string & value = values[i];

	//	table_attr.

	//	Data * data = ::toData(attr_flag, cond.value);
	//	tuple->addData(data);
	//	std::cout << "    {" << value << "}" << std::endl;
	//}

	//std::cout << "----------" << std::endl;
	//std::cout << "Table: {" << table << "}" << std::endl;
	//std::cout << "Values: " << std::endl;
	//for (const auto & value : values) {
	//	std::cout << "    {" << value << "}" << std::endl;
	//}
	//std::cout << "----------" << std::endl;
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

	catalog_manager.create_table(table, atb, primary, index, buffer);

	if (primary != -1) {
		catalog_manager.create_index(table, primary_key, primary_key, buffer);
	}


	//IndexManager index_manager;
	////bool res;
	////int i;

	////getTable
	////res = record_manager.CreateTable(tableIn);
	//index_manager

	Table * table_ptr = catalog_manager.getTable(table, buffer);
	record_manager.CreateTable(*table_ptr, buffer);
	delete table_ptr;
}

void API::dropTable(
	const std::string & table
)
{
	std::cout << "----------" << std::endl;
	std::cout << "Table: {" << table << "}" << std::endl;
}

void API::createIndex(
	const std::string & table,
	const std::string & column,
	const std::string & index
)
{
	std::cout << "----------" << std::endl;
	std::cout << "Table: {" << table << "}" << std::endl;
	std::cout << "Column: {" << column << "}" << std::endl;
	std::cout << "Index: {" << index << "}" << std::endl;
}

void API::dropIndex(
	const std::string & index
)
{
	std::cout << "----------" << std::endl;
	std::cout << "Index: {" << index << "}" << std::endl;
}

