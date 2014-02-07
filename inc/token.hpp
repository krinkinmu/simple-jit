#ifndef __TOKEN_HPP__
#define __TOKEN_HPP__

#include <cstddef>
#include <string>

#include <common.hpp>

namespace vm {


/**
 * Order is important, tokens MUST be sorted by length!
 * Not best solustion, but it simplifies scanner code little bit.
 *
 * May be rewrite with templates later?
 **/

#define FOR_PUNCTUATORS(TOKEN)	\
		TOKEN(lor, "||", 4)				\
		TOKEN(land, "&&", 5)			\
		TOKEN(eq, "==", 9)				\
		TOKEN(neq, "!=", 9)				\
		TOKEN(ge, ">=", 10)				\
		TOKEN(le, "<=", 10)				\
		TOKEN(range, "..", 9)			\
		TOKEN(incrset, "+=", 14)		\
		TOKEN(decrset, "-=", 14)		\
		TOKEN(lparen, "(", 0)			\
		TOKEN(rparen, ")", 0)			\
		TOKEN(lbrace, "{", 0)			\
		TOKEN(rbrace, "}", 0)			\
		TOKEN(assign, "=", 2)			\
		TOKEN(aor, "|", 4)				\
		TOKEN(aand, "&", 5)				\
		TOKEN(axor, "^", 5)				\
		TOKEN(lnot, "!", 0)				\
		TOKEN(gt, ">", 10)				\
		TOKEN(lt, "<", 10)				\
		TOKEN(add, "+", 12)				\
		TOKEN(sub, "-", 12)				\
		TOKEN(mul, "*", 13)				\
		TOKEN(div, "/", 13)				\
		TOKEN(mod, "%", 13)				\
		TOKEN(comma, ",", 0)			\
		TOKEN(semi, ";", 0)

#define FOR_UTILITIES(TOKEN)	\
		TOKEN(eof, "")					\
		TOKEN(ident, "")				\
		TOKEN(double_l, "")				\
		TOKEN(int_l, "")				\
		TOKEN(string_l, "")

#define FOR_KEYWORDS(KEYWORD)	\
		KEYWORD(double_t, "double")			\
		KEYWORD(int_t, "int")				\
		KEYWORD(string_t, "string")			\
		KEYWORD(for_kw, "for")				\
		KEYWORD(while_kw, "while")			\
		KEYWORD(if_kw, "if")				\
		KEYWORD(print_kw, "print")			\
		KEYWORD(function_kw, "function")	\
		KEYWORD(native_kw, "native")		\
		KEYWORD(return_kw, "return")

	class Token
	{
	public:
		enum Kind
		{
			undef,

			#define KIND(k, s, p) k,
			FOR_PUNCTUATORS(KIND)
			#undef KIND

			#define KIND(k, s) k,
			FOR_KEYWORDS(KIND)
			FOR_UTILITIES(KIND)
			#undef KIND

			token_count
		};
	
		static char const * get_token_value(Token::Kind kind) noexcept;
		static Kind get_token_kind(char const * value) noexcept;

		explicit Token(Kind kind, Location loc = Location())
			: Token(kind, get_token_value(kind), std::move(loc))
		{ }

		Token(Kind kind, std::string value, Location loc = Location())
			: kind_(kind), value_(std::move(value)), location_(std::move(loc))
		{ }

		Token(Kind kind, char const * const value, Location loc = Location())
			: kind_(kind), value_(value), location_(std::move(loc))
		{ }

		Token(Token const &) = default;
		Token & operator=(Token const &) = default;
		Token(Token &&) noexcept = default;
		//there is no noexcept here because of clang++ bug
		Token & operator=(Token && tok) = default;

		Token & swap(Token & tok) noexcept;

		Kind kind() const noexcept
		{ return kind_; }

		std::string const & value() const noexcept
		{ return value_; }

		Location const & location() const noexcept
		{ return location_; }

	private:
		Kind kind_;
		std::string value_;
		Location location_;
	};

}

#endif /*__TOKEN_HPP__*/
