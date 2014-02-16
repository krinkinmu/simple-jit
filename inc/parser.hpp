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
		std::size_t pos_;

		Token peek_token(std::size_t offset = 0) const noexcept;
		Token extract_token() noexcept;
		void consume_token(std::size_t count = 1) noexcept;
		bool ensure_token(Token::Kind kind) noexcept;

		void clear() noexcept;
		Function * parse_toplevel() noexcept;

		Status::Code parse_block(Block * block) noexcept;
	};

}

#endif /*__PARSER_HPP__*/
