#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <ast.hpp>
#include <scanner.hpp>

namespace vm
{

	class Parser
	{
	public:
		Parser();
		~Parser();

		Parser(Parser const &) = delete;
		Parser & operator=(Parser const &) = delete;

		Parser(Parser &&) noexcept = default;
		Parser & operator=(Parser &&) noexcept = default;

		Function * parse(std::string const & code) noexcept;
		Function * parse(std::string const & code, Status & status) noexcept;

	private:
		Function *top_;
		Scope *scope_;
		Status *status_;
		TokenList tokens_;

		void clear() noexcept;
		Function * parse_toplevel() noexcept;
	};

}

#endif /*__PARSER_HPP__*/
