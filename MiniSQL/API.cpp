#include "API.h"

#include <iostream>

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
	std::cout << "----------" << std::endl;
	std::cout << "Table: {" << table << "}" << std::endl;
	std::cout << "Primary Key: {" << primary_key << "}" << std::endl;
	for (const auto & attr : attrs) {
		std::string type;
		switch (attr.type) {
		case ColumnAttribute::Character:
			type = "char";
			break;
		case ColumnAttribute::Integer:
			type = "int";
			break;
		case ColumnAttribute::Float:
			type = "float";
			break;

		}

		std::cout << "    name: {" << attr.name << "}" << std::endl;
		std::cout << "    type: {" << type << "}" << std::endl;
		std::cout << "    length: {" << attr.length << "}" << std::endl;
		if (attr.unique) {
			std::cout << "    (unique)" << std::endl;
		}
		std::cout << std::endl;
	}
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

