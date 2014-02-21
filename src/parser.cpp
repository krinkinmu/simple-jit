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

	void Parser::error(std::string message, Location loc)
	{ status_->swap(Status(Status::ERROR, message, loc)); }

	bool Parser::is_ok() const noexcept
	{ return status_->code() != Status::ERROR; }

	Function * Parser::parse(std::string const & code)
	{
		Status status;
		return parse(code, status);
	}

	Function * Parser::parse(std::string const & code, Status & status)
	{
		clear();
		if (Scanner().scan(code, tokens_, status) == Status::ERROR)
			return nullptr;

		status_ = &status;

		push_scope();
		Function * top = parse_toplevel();
		if (!top)
			delete scope();
		return top;
	}

	void Parser::clear() noexcept
	{
		top_ = nullptr;
		scope_ = nullptr;
		status_ = nullptr;
		tokens_.clear();
		pos_ = 0;
	}

	Token Parser::peek_token(std::size_t offset) const
	{ return tokens_.at(pos_ + offset); }

	Token Parser::extract_token()
	{
		Token tok = peek_token();
		consume_token();
		return tok;
	}

	void Parser::consume_token(std::size_t count) noexcept
	{ pos_ += count; }

	bool Parser::ensure_token(Token::Kind kind)
	{
		if (kind == peek_token().kind())
		{
			consume_token();
			return true;
		}
		return false;
	}

	void Parser::push_scope()
	{
		scope_ = new(std::nothrow) Scope(scope_);
		assert(scope_);
	}

	void Parser::pop_scope()
	{
		assert(scope_);
		scope_ = scope_->owner();
	}

	Scope * Parser::scope() noexcept
	{ return scope_; }

	Function * Parser::parse_toplevel()
	{
		std::unique_ptr<Block> body(new(std::nothrow) Block(scope()));
		assert(body.get());

		while (peek_token().kind() != Token::eof)
		{
			if (ensure_token(Token::semi))
				continue;

			ASTNode * const node = parse_statement();
			if (!node)
				return nullptr;
			body->push_back(node);
		}

		Function * const top = new(std::nothrow) Function(Signature(Type::Void, "_start"), body.get());

		if (top)
			body.release();

		return top;
	}

	Block * Parser::parse_block()
	{
		push_scope();	
		assert(ensure_token(Token::lbrace));

		std::unique_ptr<Block> blk(new (std::nothrow) Block(scope()));
		assert(blk.get());

		while (peek_token().kind() != Token::rbrace)
		{
			if (ensure_token(Token::semi))
				continue;

			ASTNode * const stmt = parse_statement();
			if (!stmt)
				return nullptr;
			blk->push_back(stmt);
		}

		assert(ensure_token(Token::rbrace));
		pop_scope();

		return blk.release();
	}

	ASTNode * Parser::parse_statement()
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
			return parse_block();

		Token const next = peek_token(1);
		if (tok.kind() == Token::ident && Token::is_assignment(next.kind()))
			return parse_assignment();

		return parse_expression();
	}

	ASTNode * Parser::parse_assignment()
	{
		Token const var = extract_token();
		assert(var.kind() == Token::ident);

		Variable * const variable = scope()->lookup_variable(var.value());
		if (!variable)
		{
			error("unknown variable " + var.value(), var.location());
			return nullptr;
		}

		Token const op = extract_token();
		assert(Token::is_assignment(op.kind()));

		ASTNode * const expr = parse_expression();
		if (!expr)
			return nullptr;

		StoreNode * const store = new(std::nothrow) StoreNode(variable,
																expr,
																op,
																var.location(), 
																expr->finish());
		assert(store);

		return store;
	}

}
