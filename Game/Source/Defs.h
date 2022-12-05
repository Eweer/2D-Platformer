#ifndef __DEFS_H__
#define __DEFS_H__

#include "dirent.h"

#include <string>
#include <cstdio>
#include <unordered_map>
#include <variant>
#include <stdarg.h>
#include <initializer_list>
#include <ranges>
#include <algorithm>
#include <string>

// Deletes a buffer
#define RELEASE( x )		\
{							\
	if( x != nullptr )		\
	{						\
		delete x;			\
		x = nullptr;		\
	}						\
}

// Deletes an array of buffers
#define RELEASE_ARRAY( x )	\
{							\
	if( x != nullptr )		\
	{						\
		delete[] x;			\
		x = nullptr;		\
	}						\
}

#define IN_RANGE(value, min, max) ((value >= min && value <= max) ? 1 : 0)
#define TO_BOOL(a)  ((a != 0) ? true : false )

using uint = unsigned int;
using uchar = unsigned char;
using uint32 = unsigned int;
using uint64 = unsigned long long;

constexpr uint str2int(const char *str, int h = 0)
{
	return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
}

constexpr uint g_BoolTypeStr2Int = str2int("bool");
constexpr uint g_IntTypeStr2Int = str2int("int");
constexpr uint g_UIntTypeStr2Int = str2int("uint");
constexpr uint g_UInt2TypeStr2Int = str2int("unsigned int");
constexpr uint g_FloatTypeStr2Int = str2int("float");


template <class VALUE_TYPE> void SWAP(VALUE_TYPE &a, VALUE_TYPE &b)
{
	VALUE_TYPE tmp = a;
	a = b;
	b = tmp;
}

// Standard string size
constexpr auto SHORT_STR = 32;
constexpr auto MID_STR = 255;
constexpr auto HUGE_STR = 8192;

// Joins a path and file
inline const char *PATH(const char *folder, const char *file)
{
	std::string path(folder);
	path += file;
	const char *ret = path.c_str();
	return ret;
}

inline bool StrEquals(const std::string_view &lhs, const std::string_view &rhs)
{
	auto to_lower{std::ranges::views::transform(std::tolower)};
	return std::ranges::equal(lhs | to_lower, rhs | to_lower);
}

inline const char *PATH(std::string const &folder, std::string const &file)
{
	return PATH(folder.c_str(), file.c_str());
}

inline std::string PATH_STR(const char *folder, const char *file)
{
	return std::string(folder, file);
}

inline std::string PATH_STR(std::string const &folder, std::string const &file)
{
	return folder + file;
}

inline int DescAlphasort(const struct dirent **c, const struct dirent **d)
{
	return alphasort(d, c);
}

struct StringHash
{
	using is_transparent = void;
	[[nodiscard]] size_t operator()(const char *txt) const
	{
		return std::hash<std::string_view>{}(txt);
	}
	[[nodiscard]] size_t operator()(std::string_view txt) const
	{
		return std::hash<std::string_view>{}(txt);
	}
	[[nodiscard]] size_t operator()(const std::string &txt) const
	{
		return std::hash<std::string>{}(txt);
	}
};

// Performance macros
#define PERF_START(timer) timer.Start()
#define PERF_PEEK(timer) LOG("%s took %f ms", __FUNCTION__, timer.ReadMs())

// Usage: range<min, man>::contains(var)
template <int min, int max> class range
{
	static bool contains(int i)
	{ 
		return min <= i  && i < max;
	} 
};

template<typename T>
T MAX(T a, T b)
{
	return (a > b) ? a : b;
}

template <typename T>
T MAX(const T &a, const T &b, const std::initializer_list<T> &l)
{
	T max{MAX(a, b)};
	for(auto &elem : l)
	{
		max = MAX(max, elem);
	}

	return max;
}

template<typename T>
T MIN(T a, T b)
{
	return (a < b) ? a : b;
}

template <typename T>
T MIN(const T &a, const T &b, const std::initializer_list<T> &l)
{
	T min{MIN(a, b)};
	for(auto &elem : l)
	{
		min = MIN(min, elem);
	}

	return min;
}

#endif	// __DEFS_H__