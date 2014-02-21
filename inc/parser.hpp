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

		Parser(Parser const &) = default;
		Parser & operator=(Parser const &) = delete;

		Parser(Parser &&) noexcept = default;
		Parser & operator=(Parser &&) noexcept = delete;

		Function * parse(std::string const & code);
		Function * parse(std::string const & code, Status & status);

	private:
		Function *top_;
		Scope *scope_;
		Status *status_;
		TokenList tokens_;
		std::size_t pos_;

		void error(std::string message, Location loc = Location());
		bool is_ok() const noexcept;

		Token peek_token(std::size_t offset = 0) const;
		Token extract_token();
		void consume_token(std::size_t count = 1) noexcept;
		bool ensure_token(Token::Kind kind);

		void push_scope();
		void pop_scope();
		Scope * scope() noexcept;

		void clear() noexcept;
		Function * parse_toplevel();

		Block * parse_block();
		ASTNode * parse_statement();
		StoreNode * parse_assignment();
		ASTNode * parse_function();
		WhileNode * parse_while();
		ForNode * parse_for();
		IfNode * parse_if();
		ReturnNode * parse_return();
		PrintNode * parse_print();
		ASTNode * parse_declaration();
		ASTNode * parse_expression();
	};

}

#endif /*__PARSER_HPP__*/
