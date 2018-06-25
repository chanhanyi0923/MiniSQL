#include <iostream>
#include <fstream>

#include "QueryException.h"
#include "Interpreter.h"
#include "API.h"

#include "BufferBlock.h"

BufferBlock * BufferBlock::m_blocks = NULL;
BufferBlock buffer;

int main(int argc, char *argv[])
{
	BufferBlock::initiate_blocks();

	API api;
	Interpreter interpreter;
	for (bool repeat = true; repeat; ) {
		if (interpreter.getSourceType() == Interpreter::StdCin) {
			if (std::cin.eof()) {
				break;
			}

			std::cout << "minisql> ";
			std::cin >> interpreter;
			if (!interpreter.empty()) {
				try {
					repeat = interpreter.execute(&api);
				} catch (const QueryException & e) {
					std::cout << "Syntax error: " << e.what() << std::endl;
				}
				interpreter.clear();
			}
		} else if (interpreter.getSourceType() == Interpreter::File) {
			std::fstream fin;
			fin.exceptions(std::fstream::badbit);

			try {
				const std::string & filename = interpreter.getSourceFile();
				fin.open(filename.c_str(), std::fstream::in | std::fstream::binary);
				if (!fin) {
					throw std::system_error{ errno, std::generic_category() };
				}

				while (repeat) {
					if (fin.eof()) {
						interpreter.setSourceType(Interpreter::StdCin);
						break;
					}

					fin >> interpreter;
					if (!interpreter.empty()) {
						std::cout << "minisql(file)> " << interpreter.getQueryString() << std::endl;
						try {
							repeat = interpreter.execute(&api);
						} catch (const QueryException & e) {
							std::cout << "Syntax error: " << e.what() << std::endl;
						}
						interpreter.clear();
					}
				}

				fin.close();
			} catch (std::system_error const & e) {
				std::cout << "File I/O error: " << e.code() << " - " << e.what() << std::endl;
				interpreter.setSourceType(Interpreter::StdCin);
			}
		}
	}
	return 0;
}