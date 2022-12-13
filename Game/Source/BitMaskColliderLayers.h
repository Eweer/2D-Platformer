#ifndef __BITMASKCOLLIDERLAYERS_H__
#define __BITMASKCOLLIDERLAYERS_H__

namespace CL
{
	using uint16 = unsigned short;

	enum class ColliderLayers
	{
		PLATFORMS = 0x0001,
		PLAYER = 0x0002,
		ENEMIES = 0x0004,
		ITEMS = 0x0008,
		TRIGGERS = 0x0010,
		CHECKPOINTS = 0x0020,
		BULLET = 0x0040,
		UNKNOWN = 0x8000
	};

	inline ColliderLayers operator|(ColliderLayers a, ColliderLayers b)
	{
		return static_cast<ColliderLayers>(static_cast<uint16>(a) | static_cast<uint16>(b));
	}

	inline ColliderLayers operator&(ColliderLayers a, ColliderLayers b)
	{
		return static_cast<ColliderLayers>(static_cast<uint16>(a) & static_cast<uint16>(b));
	}

	inline ColliderLayers operator^(ColliderLayers a, ColliderLayers b)
	{
		return static_cast<ColliderLayers>(static_cast<uint16>(a) ^ static_cast<uint16>(b));
	}

	inline ColliderLayers operator~(ColliderLayers a)
	{
		return static_cast<ColliderLayers>(~static_cast<uint16>(a));
	}

	inline ColliderLayers &operator|=(ColliderLayers &a, ColliderLayers b)
	{
		return a = a | b;
	}

	inline ColliderLayers  operator&=(ColliderLayers  a, ColliderLayers b)
	{
		return a = a & b;
	}

	inline ColliderLayers  operator^=(ColliderLayers  a, ColliderLayers b)
	{
		return a = a ^ b;
	}

	inline bool operator !=(const ColliderLayers &a, const uint &b)
	{
		return static_cast<uint16>(a) != b;
	}

	template <ColliderLayers>
	inline bool operator ==(const ColliderLayers &a, const ColliderLayers &b)
	{
		return static_cast<uint16>(a) == static_cast<uint16>(b);
	}

}
#endif