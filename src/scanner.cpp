#include <cstring>

#include <scanner.hpp>

namespace vm
{

	TokenList::TokenList()
	{ }

	void TokenList::clear() noexcept
	{ tokens_.clear(); }

	void TokenList::push_back(Token token)
	{ tokens_.push_back(std::move(token)); }

	void TokenList::emplace_back(Token::Kind kind, std::string value, Location loc)
	{ tokens_.emplace_back(kind, std::move(value), std::move(loc)); }

	Token const & TokenList::at(size_t index) const noexcept
	{
		static Token const err(Token::undef);
		static Token const end(Token::eof);

		if (index < tokens_.size())
			return tokens_[index];

		return (index == tokens_.size()) ? end : err;
	}

	Token::Kind TokenList::kind_at(size_t index) const noexcept
	{ return at(index).kind(); }

	Location const & TokenList::location_at(size_t index) const noexcept
	{ return at(index).location(); }

	std::string const & TokenList::value_at(size_t index) const noexcept
	{ return at(index).value(); }

	namespace detail
	{

		static bool is_letter(char ch) noexcept
		{ return (('A' <= ch) && (ch <= 'Z')) || (('a' <= ch) && (ch <= 'z')) || (ch == '_'); }

		static bool is_digit(char ch) noexcept
		{ return ('0' <= ch) && (ch <= '9'); }

		static bool is_whitespace(char ch) noexcept
		{ return (ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n'); }

		static char get_unescaped(char ch) noexcept
		{
			switch (ch)
			{
			case 'n': return '\n';
			case 't': return '\t';
			case 'r': return '\r';
			case '\\': return '\\';
			case '\'': return '\'';
			}
			return ch;
		}

	}


	Scanner::Scanner()
	{ reset(); }

	Status::Code Scanner::scan(std::string const & code, TokenList & tokens, Status & status)
	{
		reset(&tokens, &status, &code);
		scan_impl();
		return status_->code();
	}

	char Scanner::peek_char(size_t off) const noexcept
	{ return (pos_ + off < code_->size()) ? code_->at(pos_ + off) : '\0'; }

	char Scanner::get_char() noexcept
	{
		char ch = peek_char();

		++offset_; ++pos_;

		if (ch == '\n')
		{
			offset_ = 0;
			++line_;
		}

		return ch;
	}

	void Scanner::skip_chars(size_t n) noexcept
	{ while (n--) get_char(); } 

	Location Scanner::current_location() const noexcept
	{ return Location(line_, offset_); }

	bool Scanner::is_ok() const noexcept
	{ return status_->code() != Status::ERROR; }

	void Scanner::error(std::string message, Location location)
	{ Status(Status::ERROR, std::move(message), std::move(location)).swap(*status_); }

	void Scanner::scan_string()
	{
		Token::Kind kind(Token::string_l);
		Location location(current_location());
		std::string value;

		get_char();
		while (peek_char() != '\0' && peek_char() != '\'')
		{
			if (peek_char() == '\\' && peek_char(1) != '\0')
			{
				get_char();
				value += detail::get_unescaped(get_char());
			}
			value += get_char();
		}

		if (peek_char() == '\'')
		{
			get_char();
			tokens_->emplace_back(kind, std::move(value), std::move(location));
			return;
		}

		error("unexpected end of file", current_location());
	}

	void Scanner::scan_number()
	{
		Token::Kind kind(Token::int_l);
		Location location(current_location());
		std::string value;

		while (detail::is_digit(peek_char()))
			value += get_char();

		if (peek_char() == 'e' || peek_char() == '.')
		{
			kind = Token::double_l;
			if (peek_char() == 'e' && (peek_char(1) == '-' || peek_char(1) == '+'))
				value += get_char();
			value += get_char();

			while (detail::is_digit(peek_char()))
				value += get_char();
		}

		tokens_->emplace_back(kind, std::move(value), std::move(location));
	}

	void Scanner::scan_ident()
	{
		Location location(current_location());
		std::string value;

		while (detail::is_letter(peek_char()) || detail::is_digit(peek_char()))
			value += get_char();

		Token::Kind kind = Token::get_token_kind(value.c_str());
		if (kind == Token::undef)
			kind = Token::ident;

		tokens_->emplace_back(kind, std::move(value), std::move(location));
	}

	void Scanner::skip_comment()
	{
		if (peek_char() == '/' && peek_char(1) == '/')
			while (peek_char() != '\0' && get_char() != '\n');
	}

	void Scanner::skip_whitespaces()
	{
		while (detail::is_whitespace(peek_char()))
			get_char();
	}

	void Scanner::scan_impl()
	{
		while (is_ok() && peek_char() != '\0')
		{
			skip_whitespaces();
			skip_comment();

			char const ch = peek_char();
			if (ch == '\0')
				continue;

			if (detail::is_letter(ch))
			{
				scan_ident();
				continue;
			}

			if (detail::is_digit(ch))
			{
				scan_number();
				continue;
			}

			if (ch == '\'')
			{
				scan_string();
				continue;
			}

			Token::Kind kind(Token::undef);
			char const val[3] = { ch, peek_char(1), '\0' };
			do
			{
				#define CASE(t, s, p)	\
					if (strlen(s) && !strncmp(s, val, strlen(s)))	\
					{ 												\
						kind = Token::t;							\
						break;										\
					}
				FOR_TOKENS(CASE)
				#undef CASE
			}
			while (0);

			if (kind == Token::undef)
			{
				error("undefined token", current_location());
				break;
			}

			Token tok(kind, current_location());
			skip_chars(tok.value().size());
			tokens_->push_back(std::move(tok));
		}
	}

	void Scanner::reset(TokenList * tokens, Status * status, std::string const * code) noexcept
	{
		line_ = 0;
		offset_ = 0;
		pos_ = 0;

		tokens_ = tokens;
		status_ = status;
		code_ = code;

		if (status_)
			Status().swap(*status_);
	}

}
