#include <cstring>
#include <cassert>

#include <token.hpp>

namespace vm
{

	char const * Token::get_token_value(Token::Kind kind) noexcept
	{
		assert(kind > Token::undef && kind < Token::token_count);

		switch (kind)
		{
		default: return "";

		#define CASE(t, s, p) case Token::t : return s;
		FOR_PUNCTUATORS(CASE)
		#undef CASE

		#define CASE(t, s) case Token::t : return s;
		FOR_KEYWORDS(CASE)
		#undef CASE
		}
		return NULL;
	}

	Token::Kind Token::get_token_kind(char const * value) noexcept
	{
		if (!value)
				return Token::undef;

		#define CASE(t, s, p) if (!strcmp(s, value)) return Token::t;
		FOR_PUNCTUATORS(CASE)
		#undef CASE

		#define CASE(t, s) if (!strcmp(s, value)) return Token::t;
		FOR_KEYWORDS(CASE)
		#undef CASE

		return Token::undef;
	}

	Token & Token::swap(Token & tok) noexcept
	{
		using std::swap;

		swap(kind_, tok.kind_);
		swap(value_, tok.value_);
		swap(location_, tok.location_);

		return *this;
	}

}

