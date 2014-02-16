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
		if (parse_block(top->body()) != Status::ERROR)
			return top.release();
		return nullptr;
	}

	Status::Code Parser::parse_block(Block * body) noexcept
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
		{
			//do warning;
		}
		return Status::SUCCESS;
	}

}
