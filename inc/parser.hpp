#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <ast.hpp>
#include <scanner.hpp>

namespace vm
{

	class Program
	{
	public:
		Program(std::unique_ptr<Function> fun, std::unique_ptr<Scope> scope) noexcept;
		~Program();

		Program(Program const &) = delete;
		Program & operator=(Program const &) = delete;

		Program(Program &&) noexcept = default;
		Program & operator=(Program &&) noexcept = default;

		Function const * top_level() const noexcept;
		Function * top_level() noexcept;

	private:
		Function * top_;
		Scope * scope_;
	};

	class Parser
	{
	public:
		Parser();
		~Parser();

		Parser(Parser const &) = default;
		Parser & operator=(Parser const &) = delete;

		Parser(Parser &&) noexcept = default;
		Parser & operator=(Parser &&) noexcept = delete;

		std::unique_ptr<Program> parse(std::string const & code);
		std::unique_ptr<Program> parse(std::string const & code, Status & status);

	private:
		Scope *scope_;
		Status *status_;
		TokenList tokens_;
		std::size_t pos_;

		void error(std::string message, Location loc = Location());
		bool is_ok() const noexcept;

		Token::Kind peek_token(std::size_t offset = 0) const noexcept;
		Location location() const noexcept;
		Token extract_token();
		void consume_token(std::size_t count = 1) noexcept;
		bool ensure_token(Token::Kind kind);

		void push_scope();
		void pop_scope();
		Scope * scope() noexcept;

		void clear() noexcept;
		std::unique_ptr<Function> parse_toplevel();

		std::unique_ptr<ASTNode> parse_binary(int precedence = 1);
		std::unique_ptr<ASTNode> parse_unary();
		std::unique_ptr<ASTNode> parse_int();
		std::unique_ptr<ASTNode> parse_double();

		std::unique_ptr<Block> parse_block();
		std::unique_ptr<ASTNode> parse_statement();
		std::unique_ptr<StoreNode> parse_assignment();
		std::unique_ptr<CallNode> parse_call();
		std::unique_ptr<WhileNode> parse_while();
		std::unique_ptr<ForNode> parse_for();
		std::unique_ptr<IfNode> parse_if();
		std::unique_ptr<ReturnNode> parse_return();
		std::unique_ptr<PrintNode> parse_print();
		std::unique_ptr<ASTNode> parse_declaration();
		std::unique_ptr<ASTNode> parse_expression();
		std::unique_ptr<Function> parse_function();
	};

}

#endif /*__PARSER_HPP__*/
