#include "API.h"


#include <iostream>


#include "BufferBlock.h"
#include "IndexManager.h"
#include "CatalogManager.h"



API::API()
{
}


API::~API()
{
}


void API::select(
	const std::string & table,
	const std::vector<std::string> & columns,
	const std::vector<Condition> & conds
)
{
	std::cout << "----------" << std::endl;
	std::cout << "Table: {" << table << "}" << std::endl;
	std::cout << "Columns: " << std::endl;

	if (columns.size() == 1 && columns.front() == "*") {
		std::cout << "All Columns." << std::endl;
	} else {
		for (const auto & col : columns) {
			std::cout << "    {" << col << "}" << std::endl;
		}
	}

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
	std::cout << "----------" << std::endl;
	std::cout << "Table: {" << table << "}" << std::endl;
	std::cout << "Values: " << std::endl;
	for (const auto & value : values) {
		std::cout << "    {" << value << "}" << std::endl;
	}
	std::cout << "----------" << std::endl;
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

