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
		Block * const body = new(std::nothrow) Block(scope());
		assert(body);

		while (peek_token() != Token::eof)
		{
			if (ensure_token(Token::semi))
				continue;

			ASTNode * const node = parse_statement();
			if (!node)
				return nullptr;
			body->push_back(node);
		}

		Function * const top = new(std::nothrow) Function(Signature(Type::Void, "_start"), body);
		assert(top);

		return top;
	}

	Block * Parser::parse_block()
	{
		push_scope();	
		assert(ensure_token(Token::lbrace));

		std::unique_ptr<Block> blk(new (std::nothrow) Block(scope()));
		assert(blk.get());

		while (peek_token() != Token::rbrace)
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

	CallNode * Parser::parse_call()
	{
		Token const fun = extract_token();
		assert(fun.kind() == Token::ident);

		std::string const & function = fun.value();
		assert(ensure_token(Token::lparen));

		std::vector<ASTNode *> args;
		while (peek_token() != Token::rparen)
		{
			ASTNode * const arg = parse_expression();
			if (!arg)
			{
				std::for_each(args.begin(), args.end(), [](ASTNode * node){ delete node; });
				return nullptr;
			}
			args->push_back(arg);

			if (!ensure_token(Token::comma) && peek_token() != Token::rparen)
			{
				std::for_each(args.begin(), args.end(), [](ASTNode * node){ delete node; });
				error("expected comma or bracket", location());
				return nullptr;
			}
		}
		assert(ensure_token(Token::rparen));

		CallNode * const call = new(std::nothrow) CallNode(function, args, fun.location(), location());
		assert(call);

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

	Function * parse_function()
	{
		assert(ensure_token(Token::function_kw));

		Token const tp = extract_token();
		if (!Token::is_typename())
		{
			error("function return type expected", tp.location());
			return nullptr;
		}

		Token const nm = extract_token();
		if (nm.kind() != Token::ident)
		{
			error("function name expected", nm.location());
			return nullptr;
		}

		if (!ensure_token(Token::lparen))
		{
			error("open bracket expected", location());
			return nullptr;
		}
		
		typedef Signature::ParametersType ParametersType;
		ParametersType parameters;
		while (peek_token() != Token::rparen)
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

			parameters.emplace_back(
					std::make_pair(
						detail::token_to_type(param_type.kind(),
							param_name.value())
						)
					);

			if (!ensure_token(Token::comma) && peek_token() != Token::rparen)
			{
				error(", or ) expected", location());
				return nullptr;
			}
		}
		assert(ensure_token(Token::rparen));

		push_scope();
		ParametersType::const_iterator const begin(parameters.begin());
		ParametersType::const_iterator const end(parameters.end());
		for (ParametersType::const_iterator it = begin(); it != end(); ++it)
		{
			Variable * const var = new (std::nothrow) Variable(it->first, it->second, tp.location(), location());
			assert(var);
			scope()->define_variable(var);
		}
		BlockNode * const body = parse_block();
		pop_scope();

		if (!body)
			return nullptr;

		Function * const fun = new(std::nothrow) Function(Signature(detail::token_to_type(tp.kind()), nm.value(), parameters), body, tp.location(), location());
		scope()->define_function(fun);

		return fun;
	}

}
