#include <cassert>
#include <vector>

#include <token.hpp>

namespace vm
{

	class TokenList
	{
	public:
		TokenList();

		TokenList(TokenList const &) = default;
		TokenList & operator=(TokenList const &) = default;
		TokenList(TokenList &&) = default;
		TokenList & operator=(TokenList &&) = default;

		void push_back(Token token);
		void emplace_back(Token::Kind kind, std::string value, Location loc);

		Token const & at(size_t index) const noexcept;
		Token::Kind kind_at(size_t index) const noexcept;

		Location const & location_at(size_t index) const noexcept;
		std::string const & value_at(size_t index) const noexcept;

		void clear() noexcept;

		template <typename Stream>
		Stream & dump(Stream & out)
		{
			for (Token const & token : tokens_)
			{
				switch (token.kind())
				{
				default: assert(0);

				#define KIND(k, s, p) case Token::k: out << #k << "\n"; break;
				FOR_TOKENS(KIND)
				#undef KIND
				}
			}
			return out;
		}

	private:
		std::vector<Token> tokens_;
	};

	class Scanner
	{
	public:
		Scanner();

		Scanner(Scanner const &) = delete;
		Scanner & operator=(Scanner const &) = delete;

		Scanner(Scanner &&) noexcept = default;
		Scanner & operator=(Scanner &&) noexcept = default;

		Status::Code scan(std::string const & code, TokenList & tokens, Status & status);

	private:
		char peek_char(size_t off = 0) const noexcept;
		char get_char() noexcept;
		void skip_chars(size_t n) noexcept;
		Location current_location() const noexcept;

		void reset(TokenList * tokens = nullptr, Status * status = nullptr, std::string const * code = nullptr) noexcept;
		bool is_ok() const noexcept;
		void error(std::string message, Location location);

		void scan_string();
		void scan_number();
		void scan_ident();
		void skip_comment();
		void skip_whitespaces();
		void scan_impl();

		size_t line_;
		size_t offset_;
		size_t pos_;
		TokenList * tokens_;
		Status * status_;
		std::string const * code_;
	};

}
