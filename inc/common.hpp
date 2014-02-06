#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <cstddef>

namespace vm
{

	class Location
	{
	public:
		static size_t const unreachable = static_cast<size_t>(-1);

		Location(size_t line = unreachable,
					size_t offset = unreachable) noexcept
			: line_(line), offset_(offset)
		{ }

		Location(Location const &) noexcept = default;
		Location & operator=(Location const &) noexcept = default;
		Location(Location &&) noexcept = default;
		Location & operator=(Location &&) noexcept = default;

		Location & swap(Location & loc) noexcept
		{
			using std::swap;

			std::swap(line_, loc.line_);
			std::swap(offset_, loc.offset_);

			return *this;
		}

		size_t line() const noexcept { return line_; }
		size_t offset() const noexcept { return offset_; }

	private:
		size_t line_;
		size_t offset_;
	};

	class Status
	{
	public:
		enum Code
		{
			ERROR,
			SUCCESS,
			NOTE,
			WARNING
		};

		Status(Code code = SUCCESS, std::string const & message = "", Location const & loc = Location())
			: code_(code), message_(message), location_(loc)
		{ }

		Status(Status const &) = default;
		Status & operator=(Status const &) = default;
		Status(Status &&) noexcept = default;
		//there is no noexcept here because of clang++ bug
		Status & operator=(Status &&) = default;

		Status & swap(Status & other) noexcept
		{
			using std::swap;

			swap(location_, other.location_);
			swap(message_, other.message_);
			swap(code_, other.code_);

			return *this;
		}

		std::string const & message() const noexcept { return message_; }
		void set_message(std::string message) noexcept { message_ = std::move(message); }

		Location const & location() const noexcept { return location_; }
		void set_location(Location loc) noexcept { location_ = std::move(loc); }

		Code code() const noexcept { return code_; }
		void set_code(Code code) noexcept { code_ = code; }

	private:
		Code code_;
		std::string message_;
		Location location_;
	};

}

#endif /*__COMMON_HPP__*/
