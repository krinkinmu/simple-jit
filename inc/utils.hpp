#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <cstddef>

namespace utils
{

	template <typename T, std::size_t N>
	constexpr std::size_t array_size(T (&)[N])
	{ return N; }

}

#endif /*__UTILS_HPP__*/
