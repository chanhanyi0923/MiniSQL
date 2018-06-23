#include "Interpreter.h"

#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/lexical_cast.hpp>
#include "QueryException.h"
#include "SqlRegex.h"


Interpreter::Interpreter():
	source_type(StdCin)
{
}


Interpreter::~Interpreter()
{
}


std::string Interpreter::getQueryString()
{
	return this->query_string;
}


void Interpreter::setSourceType(Interpreter::SourceType type)
{
	this->source_type = type;
}


Interpreter::SourceType Interpreter::getSourceType()
{
	return this->source_type;
}


std::string Interpreter::getSourceFile()
{
	return this->source_file;
}


void Interpreter::clear()
{
	this->query_string.clear();
}


bool Interpreter::empty()
{
	return this->query_string.empty();
}


std::istream & operator >> (std::istream & input, Interpreter & data)
{
	using std::vector;
	using std::string;

	string buffer;
	for (char c; (c = input.get()) != EOF && c != ';'; ) {
		if (c == '\n' || c == '\r' || c == '\t') {
			c = ' ';
		}
		buffer.push_back(c);
	}
	data.query_string = boost::trim_all_copy(buffer);

	// clear the current line
	getline(input, buffer);

	return input;
}


std::ostream & operator << (std::ostream & output, const Interpreter & data)
{
	return output;
}


bool Interpreter::execute(API * api)
{
	using std::string;
	using std::vector;

	using namespace SqlRegex;

	std::smatch result;

	/*
	 * the order of following statements is important.
	 * for example, 'select_where' must be former than 'select'.
	 */
	if (regex_search(this->query_string, result, select_where)) {
		// select $1 from $2 where $3
		vector<string> columns = Interpreter::splitByComma(result[1]);
		const string table = boost::trim_all_copy(string(result[2]));
		vector<Condition> conds = processWhere(result[3]);

		api->select(table, columns, conds);
	} else if (regex_search(this->query_string, result, select)) {
		// select $1 from $2
		vector<string> columns = Interpreter::splitByComma(result[1]);
		const string table = boost::trim_all_copy(string(result[2]));

		api->select(table, columns, vector<Condition>());
	} else if (regex_search(this->query_string, result, delete_from_where)) {
		// delete from $1 where $2
		const string table = boost::trim_all_copy(string(result[1]));
		vector<Condition> conds = Interpreter::processWhere(result[2]);

		api->deleteFrom(table, conds);
	} else if (regex_search(this->query_string, result, delete_from)) {
		// delete from $1
		const string table = boost::trim_all_copy(string(result[1]));

		api->deleteFrom(table, vector<Condition>());
	} else if (regex_search(this->query_string, result, insert)) {
		// insert into $1 values ($2)
		const string table = boost::trim_all_copy(string(result[1]));
		vector<string> values = Interpreter::splitByComma(result[2]);

		api->insert(table, values);
	} else if (regex_search(this->query_string, result, create_table)) {
		// create table $1 ($2)
		const string table = boost::trim_all_copy(string(result[1]));
		auto columns = Interpreter::processColumns(result[2]);

		api->createTable(table, columns.first, columns.second);
	} else if (regex_search(this->query_string, result, drop_table)) {
		// drop table $1
		const string table = boost::trim_all_copy(string(result[1]));

		api->dropTable(table);
	} else if (regex_search(this->query_string, result, create_index)) {
		// create index $1 on $2 ($3)
		const string index = boost::trim_all_copy(string(result[1]));
		const string table = boost::trim_all_copy(string(result[2]));
		const string column = boost::trim_all_copy(string(result[3]));

		api->createIndex(table, column, index);
	} else if (regex_search(this->query_string, result, drop_index)) {
		// drop index $1
		const string index = boost::trim_all_copy(string(result[1]));

		api->dropIndex(index);
	} else if (regex_search(this->query_string, result, execfile)) {
		// execfile $1
		const string filename = boost::trim_all_copy(string(result[1]));
		this->source_file = filename;
		this->source_type = Interpreter::File;
		

	} else if (regex_search(this->query_string, result, quit)) {
		// quit
		return false;


	} else {
		throw QueryException("Invalid operation.");
	}

	return true;
}


std::vector<std::string> Interpreter::splitByComma(const std::string & str)
{
	using std::string;
	using std::vector;

	vector<string> tokens, result;
	boost::split(tokens, str, boost::is_any_of(","), boost::token_compress_on);

	for (auto & token : tokens) {
		token = boost::trim_all_copy(token);
		if (!token.empty()) {
			result.push_back(token);
		}
	}
	return result;
}


std::vector<Condition> Interpreter::processWhere(const std::string & str)
{
	using std::string;
	using std::vector;

	vector<string> tokens;

	for (int i = 0, last = 0; i < str.size(); i++) {
		bool is_and = false;
		if (i + 2 < str.size()) {
			const char and[4] = { (char)::tolower(str[i]), (char)::tolower(str[i + 1]), (char)::tolower(str[i + 2]), 0 };
			is_and = strcmp(and, "and") == 0;
		}
		if (is_and || i + 1 == str.size()) {
			/*
			 * is and: substr(last, i - last) (without str[i], since str[i] == 'a')
			 * last token: substr(last, i + 1 - last) (with str[i])
			 */
			string token = str.substr(last, (is_and ? 0 : 1) + i - last);
			last = i + 3;

			token = boost::trim_all_copy(token);
			tokens.push_back(token);
		}
	}

	// deal with each condition
	std::vector<Condition> conditions;
	for (const string & cond_str : tokens) {
		vector<string> inner_tokens, real_inner_tokens;
		boost::split(inner_tokens, cond_str, boost::is_any_of("\t\n\r "), boost::token_compress_on);

		// remove empty tokens
		for (auto & token : inner_tokens) {
			token = boost::trim_all_copy(token);
			if (!token.empty()) {
				real_inner_tokens.push_back(token);
			}
		}

		if (real_inner_tokens.size() != 3) {
			throw QueryException("Invalid condition.");
		}

		Condition condition;
		condition.column = real_inner_tokens[0];
		const string & op = real_inner_tokens[1];
		condition.value = real_inner_tokens[2];
		
		if (op == "=") {
			condition.type = Condition::Equal;
		} else if (op == "<>") {
			condition.type = Condition::NotEqual;
		} else if (op == "<") {
			condition.type = Condition::Less;
		} else if (op == ">") {
			condition.type = Condition::Greater;
		} else if (op == "<=") {
			condition.type = Condition::LessOrEqual;
		} else if (op == ">=") {
			condition.type = Condition::GreaterOrEqual;
		} else {
			throw QueryException("Invalid condition.");
		}

		conditions.push_back(condition);
	}

	return conditions;
}


std::pair< std::string, std::vector<ColumnAttribute> > Interpreter::processColumns(const std::string & str)
{
	using std::vector;
	using std::string;
	using std::pair;

	vector<string> columns = Interpreter::splitByComma(str);
	vector<ColumnAttribute> attrs;
	string primary_key;

	for (const string & column : columns) {
		using namespace SqlRegex;

		std::smatch result;

		if (regex_search(column, result, column_primary_key)) {
			const string name = result[1];

			primary_key = name;
		} else if (regex_search(column, result, column_attr)) {
			const string name = result[1];

			string type = result[2];
			std::transform(type.begin(), type.end(), type.begin(), ::tolower);

			const string length = result[4];
			const string unique = result[5];

			ColumnAttribute attr;
			attr.name = name;
			if (type == "int") {
				attr.type = ColumnAttribute::Integer;
			} else if (type == "float") {
				attr.type = ColumnAttribute::Float;
			} else if (type == "char") {
				attr.type = ColumnAttribute::Character;
			} else {
				throw QueryException(("Unknown type '" + type + "'").c_str());
			}

			if (length.empty()) {
				attr.length = ColumnAttribute::DEFAULT_LENGTH;
			} else {
				try {
					attr.length = boost::lexical_cast<int>(length);
				} catch (...) {
					throw QueryException("the format of length is invalid.");
				}
			}
			attr.unique = !unique.empty();

			attrs.push_back(attr);
		}
	}

	return std::make_pair(primary_key, attrs);
}

