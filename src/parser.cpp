#include <memory>

#include <parser.hpp>

namespace vm
{

	Parser::Parser()
		: top_(nullptr)
		, scope_(nullptr)
	{ }

	Parser::~Parser()
	{ clear(); }

	bool Parser::is_ok() const noexcept
	{ return status_->code() != Status::ERROR; }

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
		pos_ = 0;
	}

	Token Parser::peek_token(std::size_t offset) const noexcept
	{ return tokens_.at(pos_ + offset); }

	Token Parser::extract_token() noexcept
	{
		Token tok = peek_token();
		consume_token();
		return tok;
	}

	void Parser::consume_token(std::size_t count) noexcept
	{ pos_ += count; }

	bool Parser::ensure_token(Token::Kind kind) noexcept
	{
		if (kind == peek_token().kind())
		{
			consume_token();
			return true;
		}
		return false;
	}

	Function * Parser::parse_toplevel() noexcept
	{
		Signature signature(Type::Void, "start_");
		std::unique_ptr<Function> top(new Function(signature, nullptr));
		parse_block(top->body());
		if (is_ok())
			return top.release();
		return nullptr;
	}

	void Parser::parse_block(Block * body) noexcept
	{
		scope_ = body->scope();
		bool const brace = ensure_token(Token::lbrace);

		Token::Kind const stop = brace ? Token::rbrace : Token::eof;
		for (Token tok = peek_token(); tok.kind() != stop; tok = peek_token())
		{
			if (tok.kind() == Token::semi)
			{
				consume_token();
				continue;
			}
		}

		if (brace && !ensure_token(Token::rbrace))
		{ }
	}

	ASTNode * Parser::parse_statement() noexcept
	{
		Token const tok = peek_token();
		if (Token::is_keyword(tok.kind()))
		{
			switch (tok.kind())
			{
			default: assert(0);
			case Token::function_kw:
				return parse_function();
			case Token::if_kw:
				return parse_if();
			case Token::for_kw:
				return parse_for();
			case Token::while_kw:
				return parse_while();
			case Token::print_kw:
				return parse_print();
			case Token::return_kw:
				return parse_return();
			case Token::int_t:
			case Token::double_t:
			case Token::string_t:
				return parse_declaration();
			}
		}

		if (tok.kind() == Token::lbrace)
		{
			Block * block = new(std::nothrow) Block(scope_);
			if (block)
				parse_block(block);
			return block;
		}

		Token const next = peek_token(1);
		if (tok.kind() == Token::ident && Token::is_assignment(next.kind()))
			return parse_assignment();

		return parse_expression();
	}

}
