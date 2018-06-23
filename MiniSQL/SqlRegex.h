#pragma once

#include <string>

using std::string;

namespace SqlRegex
{
	namespace Keyword
	{
		const string create("[cC][rR][eE][aA][tT][eE]");
		const string delete_("[dD][eE][lL][eE][tT][eE]");
		const string drop("[dD][rR][oO][pP]");
		const string execfile("[eE][xX][eE][cC][fF][iI][lL][eE]");
		const string from("[fF][rR][oO][mM]");
		const string index("[iI][nN][dD][eE][xX]");
		const string insert("[iI][nN][sS][eE][rR][tT]");
		const string into("[iI][nN][tT][oO]");
		const string on("[oO][nN]");
		const string quit("[qQ][uU][iI][tT]");
		const string select("[sS][eE][lL][eE][cC][tT]");
		const string table("[tT][aA][bB][lL][eE]");
		const string values("[vV][aA][lL][uU][eE][sS]");
		const string where("[wW][hH][eE][rR][eE]");


		const string unique("[uU][nN][iI][qQ][uU][eE]");
		const string primary("[pP][rR][iI][mM][aA][rR][yY]");
		const string key("[kK][eE][yY]");
	};

	const std::regex select_where(
		"^" +
		Keyword::select + "\\s+" + "(.*?)" + "\\s+" +
		Keyword::from + "\\s+" + "(.*?)" + "\\s+" +
		Keyword::where + "\\s+" + "(.*)" +
		"$"
	);

	const std::regex select(
		"^" +
		Keyword::select + "\\s+" + "(.*?)" + "\\s+" +
		Keyword::from + "\\s+" + "(.*)" +
		"$"
	);

	const std::regex delete_from_where(
		"^" +
		Keyword::delete_ + "\\s+" +
		Keyword::from + "\\s+" + "(.*?)" + "\\s+" +
		Keyword::where + "\\s+" + "(.*)" +
		"$"
	);

	const std::regex delete_from(
		"^" +
		Keyword::delete_ + "\\s+" +
		Keyword::from + "\\s+" + "(.*)" +
		"$"
	);

	const std::regex insert(
		"^" +
		Keyword::insert + "\\s+" + Keyword::into + "\\s+" +
		"(.*?)" + "\\s+" +
		Keyword::values + "\\s*" +
		"\\(" + "(.*?)" + "\\)" +
		"$"
	);

	const std::regex create_table(
		"^" +
		Keyword::create + "\\s+" + Keyword::table + "\\s+" +
		"(.*?)" + "\\s*" +
		"\\(" + "(.*?)" + "\\)" +
		"$"
	);

	const std::regex drop_table(
		"^" +
		Keyword::drop + "\\s+" + Keyword::table + "\\s+" +
		"(.*)" +
		"$"
	);

	const std::regex create_index(
		"^" +
		Keyword::create + "\\s+" + Keyword::index + "\\s+" +
		"(.*?)" + "\\s+" +
		Keyword::on + "\\s+" + "(.*?)" +
		"\\s*" + "\\(" + "(.*?)" + "\\)" +
		"$"
	);

	const std::regex drop_index(
		"^" +
		Keyword::drop + "\\s+" + Keyword::index + "\\s+" +
		"(.*)" +
		"$"
	);

	const std::regex execfile(
		"^" +
		Keyword::execfile + "\\s+" +
		"(.*)" +
		"$"
	);

	const std::regex quit(
		"^" + Keyword::quit + "$"
	);

	const std::regex column_attr(
		std::string("^") +
		"(.*?)" + "\\s+" +
		"([a-zA-Z]*)" + "\\s*" +
		"(\\(" + "([0-9]+)" + "\\))?" + "\\s*"
		"(" + Keyword::unique + ")?" +
		"$"
	);

	const std::regex column_primary_key(
		"^" +
		Keyword::primary + "\\s+" +
		Keyword::key + "\\s*" +
		"\\(" + "\\s*" + "(.*?)" + "\\s*" + "\\)" + "\\s*" +
		"$"
	);
};

