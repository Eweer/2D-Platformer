#ifndef __BITMASK_H__
#define __BITMASK_H__

#include "Defs.h"

using uint16 = unsigned short;

enum class ColliderLayers
{
	PLATFORMS = 0x0001,
	PLAYER = 0x0002,
	ENEMIES = 0x0004,
	ITEMS = 0x0008,
	TRIGGERS = 0x0010,
	CHECKPOINTS = 0x0020
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

#endif