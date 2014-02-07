#include <cstring>
#include <cassert>

#include <token.hpp>

namespace vm
{

	static char const *token_values[] = {
		#define VALUE(k, s, p) s,
		FOR_TOKENS(VALUE)
		#undef VALUE
		""
	};

	char const * Token::get_token_value(Token::Kind kind) noexcept
	{
		assert(kind > Token::undef && kind < Token::token_count);

		return token_values[static_cast<size_t>(kind)];
	}

	Token::Kind Token::get_token_kind(char const * value) noexcept
	{
		if (!value || !strcmp(value, ""))
				return Token::undef;

		#define KIND(t, s, p) if (!strcmp(s, value)) return Token::t;
		FOR_TOKENS(KIND)
		#undef KIND

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

