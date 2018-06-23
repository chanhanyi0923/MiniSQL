#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "Condition.h"
#include "ColumnAttribute.h"
#include "API.h"

class Interpreter
{
public:
	enum SourceType { StdCin, File };

private:
	SourceType source_type;
	std::string source_file;

	// query string (tokens)
	std::string query_string;

	static std::pair< std::string, std::vector<ColumnAttribute> > processColumns(const std::string & str);
	static std::vector<Condition> processWhere(const std::string & str);
	static std::vector<std::string> splitByComma(const std::string & str);

public:
	Interpreter();
	virtual ~Interpreter();

	std::string getQueryString();
	bool empty();
	void clear();
	void setSourceType(SourceType type);
	SourceType getSourceType();
	std::string getSourceFile();
	bool execute(API * api);

	friend std::istream & operator >> (std::istream & input, Interpreter & data);
	friend std::ostream & operator << (std::ostream & output, const Interpreter & data);
};

