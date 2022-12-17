#ifndef __BITMASKNAVTYPE_H__
#define __BITMASKNAVTYPE_H__

namespace CL
{
	using uint16 = unsigned short;
		
	enum class NavType
	{
		NONE = 0x0000,
		SOLO = 0x0001,
		LEFT = 0x0002,
		RIGHT = 0x0004,
		PLATFORM = 0x0008,
		TERRAIN = 0x0010
	};

	inline NavType operator|(NavType a, NavType b)
	{
		return static_cast<NavType>(static_cast<uint16>(a) | static_cast<uint16>(b));
	}

	inline NavType operator&(NavType a, NavType b)
	{
		return static_cast<NavType>(static_cast<uint16>(a) & static_cast<uint16>(b));
	}

	inline NavType operator^(NavType a, NavType b)
	{
		return static_cast<NavType>(static_cast<uint16>(a) ^ static_cast<uint16>(b));
	}

	inline NavType operator~(NavType a)
	{
		return static_cast<NavType>(~static_cast<uint16>(a));
	}

	inline NavType &operator|=(NavType &a, NavType b)
	{
		return a = a | b;
	}

	inline NavType  operator&=(NavType  a, NavType b)
	{
		return a = a & b;
	}

	inline NavType  operator^=(NavType  a, NavType b)
	{
		return a = a ^ b;
	}

	inline bool operator !=(const NavType &a, const uint16 &b)
	{
		return static_cast<uint16>(a) != b;
	}

	template <NavType>
	inline bool operator ==(const NavType &a, const NavType &b)
	{
		return static_cast<uint16>(a) == static_cast<uint16>(b);
	}

}
#endif // __BITMASKNAVTYPE_H__