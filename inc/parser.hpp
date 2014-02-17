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

		bool is_ok() const noexcept;
		Token peek_token(std::size_t offset = 0) const noexcept;
		Token extract_token() noexcept;
		void consume_token(std::size_t count = 1) noexcept;
		bool ensure_token(Token::Kind kind) noexcept;

		void clear() noexcept;
		Function * parse_toplevel() noexcept;

		void parse_block(Block * block) noexcept;
		ASTNode * parse_statement() noexcept;
		StoreNode * parse_assignment() noexcept;
		ASTNode * parse_function() noexcept;
		WhileNode * parse_while() noexcept;
		ForNode * parse_for() noexcept;
		IfNode * parse_if() noexcept;
		ReturnNode * parse_return() noexcept;
		PrintNode * parse_print() noexcept;
		ASTNode * parse_declaration() noexcept;
		ASTNode * parse_expression() noexcept;
	};

}

#endif /*__PARSER_HPP__*/
