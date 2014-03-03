#include <memory>

#include <parser.hpp>

namespace vm
{

	Program::Program(std::unique_ptr<Function> fun, std::unique_ptr<Scope> scope) noexcept;
		: top_(fun.release())
		, scope_(scope.release())
	{ }

	Program::~Program()
	{
		delete top_;
		delete scope_;
	}

	Function const * Program::top_level() const noexcept
	{ return top_; }

	Function * Program::top_level() noexcept
	{ return top_; }


	Parser::Parser()
		: scope_(nullptr)
	{ }

	Parser::~Parser()
	{ clear(); }

	void Parser::error(std::string message, Location loc)
	{ status_->swap(Status(Status::ERROR, message, loc)); }

	bool Parser::is_ok() const noexcept
	{ return status_->code() != Status::ERROR; }

	std::unique_ptr<Program> Parser::parse(std::string const & code)
	{
		Status status;
		return parse(code, status);
	}

	std::unique_ptr<Program> Parser::parse(std::string const & code, Status & status)
	{
		clear();
		if (Scanner().scan(code, tokens_, status) == Status::ERROR)
			return nullptr;
		status_ = &status;

		push_scope();
		std::unique_ptr<Scope> top_scope = scope();
		std::unique_ptr<Function> top = parse_toplevel();
		pop_scope();

		return new Program(top, top_scope);
	}

	void Parser::clear() noexcept
	{
		scope_ = nullptr;
		status_ = nullptr;
		tokens_.clear();
		pos_ = 0;
	}

	Token::Kind Parser::peek_token(std::size_t offset) const noexcept
	{ return tokens_.at(pos_ + offset).kind(); }

	Location Parser::location() const noexcept
	{ return tokens_.at(pos_).location(); }

	Token Parser::extract_token()
	{
		Token tok = tokens_.at(pos_);
		consume_token();
		return tok;
	}

	void Parser::consume_token(std::size_t count) noexcept
	{ pos_ += count; }

	bool Parser::ensure_token(Token::Kind kind)
	{
		if (kind == peek_token())
		{
			consume_token();
			return true;
		}
		return false;
	}

	void Parser::push_scope()
	{ scope_ = new Scope(scope_); }

	void Parser::pop_scope()
	{
		assert(scope_);
		scope_ = scope_->owner();
	}

	Scope * Parser::scope() noexcept
	{ return scope_; }

	std::unique_ptr<Function> Parser::parse_toplevel()
	{
		std::unique_ptr<Block> body = new Block(scope());
		while (peek_token() != Token::eof)
		{
			if (ensure_token(Token::semi))
				continue;

			std::unique_ptr<ASTNode> node = parse_statement();
			if (!node)
				return nullptr;
			body->push_back(node);
		}

		std::unique_ptr<Signature> sign = new Signature(Type::Void, "_start");
		std::unique_ptr<Function> top = new Function(sign, body);

		return top;
	}

	std::unique_ptr<Block> Parser::parse_block()
	{
		push_scope();	
		assert(ensure_token(Token::lbrace));

		std::unique_ptr<Block> blk(new Block(scope()));
		while (peek_token() != Token::rbrace)
		{
			if (ensure_token(Token::semi))
				continue;

			std::unique_ptr<ASTNode> stmt = parse_statement();
			if (!stmt)
				return nullptr;
			blk->push_back(stmt);
		}

		assert(ensure_token(Token::rbrace));
		pop_scope();

		return blk;
	}

	std::unique_ptr<ASTNode> Parser::parse_statement()
	{
		Token::Kind const tok = peek_token();
		if (Token::is_keyword(tok))
		{
			switch (tok)
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

		if (tok == Token::lbrace)
			return parse_block();

		Token::Kind const next = peek_token(1);
		if (tok == Token::ident && Token::is_assignment(next))
			return parse_assignment();

		return parse_expression();
	}

	std::unique_ptr<ASTNode> Parser::parse_assignment()
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

		std::unique_ptr<ASTNode> expr = parse_expression();
		if (!expr)
			return nullptr;

		return new StoreNode(variable, expr, op, var.location(), expr->finish());
	}

	std::unique_ptr<CallNode> Parser::parse_call()
	{
		Token const fun = extract_token();
		assert(fun.kind() == Token::ident);

		std::string const & function = fun.value();
		assert(ensure_token(Token::lparen));

		std::unique_ptr<CallNode> call = new CallNode(function, fun.location());
		while (!ensure_token(Token::rparen))
		{
			std::unique_ptr<ASTNode> arg = parse_expression();
			if (!arg)
				return nullptr;
			call->push_back(arg);

			if (!ensure_token(Token::comma) && peek_token() != Token::rparen)
			{
				error("expected comma or bracket", location());
				return nullptr;
			}
		}
		call->set_finish(location());

		return call;
	}

	namespace detail
	{
		Type token_to_type(Token::Kind kind)
		{
			switch (kind)
			{
			default: assert(0);
			case Token::double_t: return Type::Double;
			case Token::int_t: return Type::Int;
			case Token::string_t: return Type::String;
			case Token::void_t: return Type::Void;
			}
			return Type::Invalid;
		}
	}

	std::unique_ptr<Function> Parser::parse_function()
	{
		assert(ensure_token(Token::function_kw));

		Token const tp = extract_token();
		if (!Token::is_typename())
		{
			error("type expected", tp.location());
			return nullptr;
		}

		Token const nm = extract_token();
		if (nm.kind() != Token::ident)
		{
			error("indentifier expected", nm.location());
			return nullptr;
		}

		if (!ensure_token(Token::lparen))
		{
			error("( expected", location());
			return nullptr;
		}

		std::unique_ptr<Signature> sign = new Signature(detail::token_to_type(tp.kind()), nm.value());
		while (!ensure_token(Token::rparen))
		{
			Token const param_type = extract_token();
			if (!Token::is_typename(param_type.kind()))
			{
				error("typename or ) expected", param_type.location());
				return nullptr;
			}

			Token const param_name = extract_token();
			if (param_name.kind() != Token::ident)
			{
				error("identifier or ) expected", param_name.location());
				return nullptr;
			}

			sign->push_back(
					std::make_pair(
							detail::token_to_type(param_type.kind()),
							param_name.value()
						)
					);

			if (!ensure_token(Token::comma) && peek_token() != Token::rparen)
			{
				error(", or ) expected", location());
				return nullptr;
			}
		}

		push_scope();

		ParametersType::const_iterator const begin(sign->begin());
		ParametersType::const_iterator const end(sign->end());
		for (ParametersType::const_iterator it = begin(); it != end(); ++it)
		{
			std::unique_ptr<Variable> var = new Variable(it->first, it->second, tp.location(), location());
			scope()->define_variable(var);
		}
		std::unique_ptr<Block> body = parse_block();
		if (!body)
			return nullptr;

		pop_scope();

		std::unique_ptr<Function> fun = new Function(sign, body, tp.location(), location());
		scope()->define_function(fun);

		return fun;
	}

	std::unique_ptr<WhileNode> Parser::parse_while()
	{
		Location const start = location();

		assert(ensure_token(Token::while_kw));

		if (!ensure_token(Token::lparen))
		{
			error("( expected", location());
			return nullptr;
		}

		std::unique_ptr<ASTNode> expr = parse_expression();
		if (!expr)
			return nullptr;

		if (!ensure_token(Token::rparen))
		{
			error(") expected", location());
			return nullptr;
		}

		std::unique_ptr<Block> body = parse_block();
		if (!body)
			return nullptr;

		return new WhileNode(expr, body, start, location());
	}

	std::unique_ptr<ForNode> Parser::parse_for()
	{
		Location const start = location();

		assert(ensure_token(Token::for_kw));

		if (!ensure_token(Token::lparen))
		{
			error("( expected", location());
			return nullptr;
		}

		Token const var = extract_token();
		if (var.kind() != Token::ident)
		{
			error("identifier expected", var.location());
			return nullptr;
		}

		if (!ensure_token(Token::in_kw))
		{
			error("in expected", location());
			return nullptr;
		}

		std::unique_ptr<ASTNode> expr = parse_expression();
		if (!expr)
			return nullptr;

		if (!ensure_token(Token::rparen))
		{
			error(") expected", location());
			return nullptr;
		}

		std::unique_ptr<Block> body = parse_block();
		if (!body)
			return nullptr;

		Variable const * const v = scope()->lookup_variable(var.value());
		if (!v)
		{
			error("unknown variable" + var.value(), var.location());
			return nullptr;
		}

		return new ForNode(v, expr, body, start, location());
	}

	std::unique_ptr<IfNode> Parser::parse_if()
	{
		Location const start = location();

		assert(ensure_token(Token::if_kw));

		if (!ensure_token(Token::lparen))
		{
			error("( expected", location());
			return nullptr;
		}

		std::unique_ptr<ASTNode> expr = parse_exception();

		if (!ensure_token(Token::rparen))
		{
			error("( expected", location());
			return nullptr;
		}

		std::unique_ptr<Block> then_body = parse_block();
		if (!then_body)
			return nullptr;

		std::unique_ptr<Block> else_body = nullptr;
		if (ensure_token(Token::else_kw))
		{
			else_body.reset(parse_block());
			if (!else_body)
				return nullptr;
		}

		return new IfNode(expr, then_body, else_body, start, location());
	}

	std::unique_ptr<ReturnNode> Parser::parse_return()
	{
		Location const loc = location();

		assert(ensure_token(Token::return_kw));

		if (ensure_token(Token::semi))
			return new ReturnNode(loc, loc);

		std::unique_ptr<ReturnNode> ret = parse_expression();
		if (!ensure_token(Token::semi))
		{
			error("; expected", location());
			return nullptr;
		}

		return new ReturnNode(ret, loc, location());
	}

	std::unique_ptr<PrintNode> Parser::parse_print()
	{
		Location const loc = location();

		assert(ensure_token(Token::return_kw));

		if (!ensure_token(Token::lparen))
		{
			error("( expected", location());
			return nullptr;
		}

		std::unique_ptr<PrintNode> print = new PrintNode(loc);
		while (!ensure_token(Token::rparen))
		{
			std::unique_ptr<ASTNode> expr = parse_expression();
			print->push_back(expr);
			if (!ensure_token(Token::comma) && peek_token() != Token::rparen)
			{
				error(", or ) expected", location());
				return nullptr;
			}
		}
		print->set_finish(location());

		if (!ensure_token(Token::semi))
		{
			error("; expected", location());
			return nullptr;
		}

		return print;
	}

}
