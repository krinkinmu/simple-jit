#include <parser.hpp>

namespace vm
{

	Parser::Parser()
		: top_(nullptr)
		, scope_(nullptr)
	{ }

	Parser::~Parser()
	{ clear(); }

	Function * Parser::parse(std::string const & code) noexcept
	{
		Status status;
		return parse(code, status);
	}

	Function * Parser::parse(std::string const & code, Status & status) noexcept
	{
		clear();
		if (Scanner().scan(code, tokens_, status) == Status::ERROR)
			return nullptr;

		status_ = &status;
		return parse_toplevel();
	}

	void Parser::clear() noexcept
	{
		top_ = nullptr;
		scope_ = nullptr;
		status_ = nullptr;
		tokens_.clear();
	}

	Function * Parser::parse_toplevel() noexcept
	{ return nullptr; }

}
