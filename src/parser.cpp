#include <cstdlib>
#include <memory>

#include <parser.hpp>

namespace vm
{

	Program::Program(std::unique_ptr<Function> fun, std::unique_ptr<Scope> scope) noexcept
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
	{ Status(Status::ERROR, message, loc).swap(*status_); }

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
		std::unique_ptr<Scope> top_scope(scope());
		std::unique_ptr<Function> top(parse_toplevel());
		pop_scope();

		return std::unique_ptr<Program>(new Program(std::move(top), std::move(top_scope)));
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
		std::unique_ptr<Block> body(new Block(scope()));
		while (peek_token() != Token::eof)
		{
			if (ensure_token(Token::semi))
				continue;

			std::unique_ptr<ASTNode> node = parse_statement();
			if (!is_ok())
				return nullptr;

			if (node)
				body->push_back(std::move(node));
		}

		std::unique_ptr<Signature> sign(new Signature(Type::Void, "_start"));

		return std::unique_ptr<Function>(new Function(std::move(sign), std::move(body)));
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

			if (peek_token() == Token::function_kw)
			{
				std::unique_ptr<Function> fun = parse_function();
				if (!fun)
					return nullptr;

				scope()->define_function(std::move(fun));
				continue;
			}

			std::unique_ptr<ASTNode> stmt = parse_statement();
			if (!is_ok())
				return nullptr;

			if (stmt)
				blk->push_back(std::move(stmt));
		}

		pop_scope();

		if (!ensure_token(Token::rbrace))
		{
			error("} expected", location());
			return nullptr;
		}

		return blk;
	}

	std::unique_ptr<ASTNode> Parser::parse_statement()
	{
		Token::Kind const tok = peek_token();
		if (Token::is_keyword(tok))
		{
			switch (tok)
			{
			default:
				error("unexpected token", location());
				return nullptr;
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

	std::unique_ptr<StoreNode> Parser::parse_assignment()
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

		return std::unique_ptr<StoreNode>(new StoreNode(variable, std::move(expr), op.kind(), var.location(), expr->finish()));
	}

	std::unique_ptr<CallNode> Parser::parse_call()
	{
		Token const fun = extract_token();
		assert(fun.kind() == Token::ident);

		std::string const & function = fun.value();
		if (!ensure_token(Token::lparen))
		{
			error("( expected", location());
			return nullptr;
		}

		std::unique_ptr<CallNode> call(new CallNode(function, fun.location()));
		while (!ensure_token(Token::rparen))
		{
			std::unique_ptr<ASTNode> arg = parse_expression();
			if (!arg)
				return nullptr;
			call->push_back(std::move(arg));

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
		if (!Token::is_typename(tp.kind()))
		{
			error("type expected", tp.location());
			return nullptr;
		}

		Token const nm = extract_token();
		if (nm.kind() != Token::ident)
		{
			error("identifier expected", nm.location());
			return nullptr;
		}

		if (!ensure_token(Token::lparen))
		{
			error("( expected", location());
			return nullptr;
		}

		std::unique_ptr<Signature> sign(new Signature(detail::token_to_type(tp.kind()), nm.value()));
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

		typedef Signature::ParametersType ParametersType;
		ParametersType::const_iterator const begin(sign->begin());
		ParametersType::const_iterator const end(sign->end());
		for (ParametersType::const_iterator it = begin; it != end; ++it)
		{
			std::unique_ptr<Variable> var(new Variable(it->first, it->second, tp.location(), location()));
			scope()->define_variable(std::move(var));
		}
		std::unique_ptr<Block> body = parse_block();
		if (!body)
			return nullptr;

		pop_scope();

		return std::unique_ptr<Function>(new Function(std::move(sign), std::move(body), tp.location(), location()));
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

		return std::unique_ptr<WhileNode>(new WhileNode(std::move(expr), std::move(body), start, location()));
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

		Variable * const v = scope()->lookup_variable(var.value());
		if (!v)
		{
			error("unknown variable" + var.value(), var.location());
			return nullptr;
		}

		return std::unique_ptr<ForNode>(new ForNode(v, std::move(expr), std::move(body), start, location()));
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

		std::unique_ptr<ASTNode> expr = parse_expression();
		if (!expr)
			return nullptr;

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
			else_body.reset(parse_block().release());
			if (!else_body)
				return nullptr;
		}

		return std::unique_ptr<IfNode>(new IfNode(std::move(expr), std::move(then_body), std::move(else_body), start, location()));
	}

	std::unique_ptr<ReturnNode> Parser::parse_return()
	{
		Location const loc = location();

		assert(ensure_token(Token::return_kw));

		if (peek_token() == Token::semi)
			return std::unique_ptr<ReturnNode>(new ReturnNode(nullptr, loc, loc));

		std::unique_ptr<ASTNode> ret = parse_expression();
		if (!ret)
			return nullptr;

		return std::unique_ptr<ReturnNode>(new ReturnNode(std::move(ret), loc, location()));
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

		std::unique_ptr<PrintNode> print(new PrintNode(loc));
		while (!ensure_token(Token::rparen))
		{
			std::unique_ptr<ASTNode> expr = parse_expression();
			if (!expr)
				return nullptr;

			print->push_back(std::move(expr));
			if (!ensure_token(Token::comma) && peek_token() != Token::rparen)
			{
				error(", or ) expected", location());
				return nullptr;
			}
		}
		print->set_finish(location());
		return print;
	}

	std::unique_ptr<ASTNode> Parser::parse_declaration()
	{
		Type type = detail::token_to_type(extract_token().kind());
		assert(type != Type::Invalid && type != Type::Void);

		Token const name = extract_token();
		if (name.kind() != Token::ident)
		{
			error("identifier expected", name.location());
			return nullptr;
		}

		std::unique_ptr<Variable> variable(new Variable(type, name.value(), name.location(), name.location()));
		Variable * ptr = variable.get();

		if (!ensure_token(Token::assign))
			return nullptr;

		std::unique_ptr<ASTNode> expr = parse_expression();
		if (!expr)
			return nullptr;

		scope()->define_variable(std::move(variable));

		return std::unique_ptr<StoreNode>(new StoreNode(ptr, std::move(expr), Token::assign, name.location(), location()));
	}

	std::unique_ptr<ASTNode> Parser::parse_expression()
	{ return parse_binary(); }

	std::unique_ptr<ASTNode> Parser::parse_binary(int prev)
	{
		std::unique_ptr<ASTNode> left = parse_unary();
		if (!left)
			return nullptr;

		int prec = Token::get_precedence(peek_token());
		if (prec <= 0)
		{
			error("operator expected", location());
			return nullptr;
		}

		while (prec >= prev)
		{
			while (Token::get_precedence(peek_token()) == prec)
			{
				Token const op = extract_token();
				std::unique_ptr<ASTNode> right = parse_binary();
				if (!right)
					return nullptr;
				left.reset(new BinaryExprNode(op.kind(), std::move(left), std::move(right), left->start(), right->finish()));
			}
			prec = Token::get_precedence(peek_token());
		}

		return left;
	}

	namespace detail
	{

		bool is_unary(Token::Kind kind) noexcept
		{ return kind == Token::lnot || kind == Token::sub; }

	}

	std::unique_ptr<ASTNode> Parser::parse_unary()
	{
		if (detail::is_unary(peek_token()))
		{
			Token const op = extract_token();
			std::unique_ptr<ASTNode> expr = parse_unary();
			if (!expr)
				return nullptr;
			return std::unique_ptr<ASTNode>(new UnaryExprNode(op.kind(), std::move(expr), op.location(), expr->finish()));
		}

		if (peek_token() == Token::ident && peek_token(1) == Token::lparen)
			return parse_call();

		if (peek_token() == Token::ident)
		{
			Token const name = extract_token();
			Variable * var = scope()->lookup_variable(name.value());
			if (!var)
			{
				error("undefined variable", name.location());
				return nullptr;
			}
			return new LoadNode(var, name.location(), name.location());
		}

		if (peek_token() == Token::double_l)
			return parse_double();

		if (peek_token() == Token::int_l)
			return parse_int();

		if (peek_token() == Token::string_l)
		{
			Token const tok = extract_token();
			return std::unique_ptr<ASTNode>(new StringLitNode(tok.value(), tok.location(), tok.location()));
		}

		if (ensure_token(Token::lparen))
		{
			std::unique_ptr<ASTNode> expr = parse_expression();
			if (!expr)
				return nullptr;

			if (!ensure_token(Token::rparen))
			{
				error(") expected", location());
				return nullptr;
			}

			return expr;
		}

		error("unexpected token", location());
		return nullptr;
	}

	std::unique_ptr<ASTNode> Parser::parse_int()
	{
		Token const tok = extract_token();
		assert(tok.kind() == Token::int_l);

		std::string const & value = tok.value();
		char * endptr = nullptr;

		long int num = strtol(value.c_str(), &endptr, 10);
		if (endptr == nullptr || *endptr != '\0')
		{
			error("integer literal expected", tok.location());
			return nullptr;
		}

		return new IntLitNode(num, tok.location(), tok.location());
	}

	std::unique_ptr<ASTNode> Parser::parse_double()
	{
		Token const tok = extract_token();
		assert(tok.kind() == Token::double_l);

		std::string const & value = tok.value();
		char * endptr = nullptr;

		double num = strtod(value.c_str(), &endptr);
		if (endptr == nullptr || *endptr != '\0')
		{
			error("double literal expected", tok.location());
			return nullptr;
		}

		return new DoubleLitNode(num, tok.location(), tok.location());
	}

}
