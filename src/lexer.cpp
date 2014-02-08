#include <algorithm>
#include <iostream>
#include <iterator>
#include <fstream>
#include <iomanip>
#include <vector>

#include <scanner.hpp>

static bool read_file(char const * file_name, std::string & code)
{
	std::vector<char> data;
	std::ifstream input(file_name);

	if (!input)
		return false;

	input >> std::noskipws;

	std::istream_iterator<char> begin(input), end;

	std::copy(begin, end, std::back_inserter(data));
	std::string(data.cbegin(), data.cend()).swap(code);

	return true;
}

int main(int argc, char **argv)
{
	for (int index = 1; index != argc; ++index)
	{
		std::string code;
		vm::TokenList tokens;
		vm::Status status;

		if (!read_file(argv[index], code))
		{
			std::cout << "ERROR: cannot read file " << argv[index] << std::endl;
			return 0;
		}

		if (!vm::Scanner().scan(code, tokens, status))
		{
			std::cout << "ERROR(" << status.location().line()
						<< ":" << status.location().offset() << "): "
						<< status.message() << std::endl;
			tokens.dump(std::cout);
			return 0;
		}

		tokens.dump(std::cout);
	}

	return 0;
}
