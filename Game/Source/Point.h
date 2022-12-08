#ifndef __POINT_H__
#define __POINT_H__

#include "Defs.h"

#include <math.h>

template<class TYPE>
class Point
{
public:

	TYPE x = 0;
	TYPE y = 0;

	Point& Create(const TYPE& a, const TYPE& b)
	{
		this->x = a;
		this->y = b;

		return *this;
	}

	// Math ------------------------------------------------
	Point operator -(const Point &v) const
	{
		Point r;

		r.x = x - v.x;
		r.y = y - v.y;

		return r;
	}

	Point operator + (const Point &v) const
	{
		Point r;

		r.x = x + v.x;
		r.y = y + v.y;

		return r;
	}

	Point operator *(uint i)
	{
		Point r;

		r.x = x * i;
		r.y = y * i;

		return r;
	}

	const Point& operator -=(const Point &v)
	{
		x -= v.x;
		y -= v.y;

		return *this;
	}

	const Point& operator +=(const Point &v)
	{
		x += v.x;
		y += v.y;

		return *this;
	}

	auto operator <=>(const Point &v) const = default;

	// Utils ------------------------------------------------
	bool IsZero() const
	{
		return (x == 0 && y == 0);
	}

	Point& SetToZero()
	{
		x = y = 0;
		return(*this);
	}

	Point& Negate()
	{
		x = -x;
		y = -y;

		return(*this);
	}

	// Distances ---------------------------------------------
	TYPE DistanceTo(const Point& v) const
	{
		TYPE fx = x - v.x;
		TYPE fy = y - v.y;

		return sqrtf((fx*fx) + (fy*fy));
	}

	TYPE DistanceNoSqrt(const Point& v) const
	{
		TYPE fx = x - v.x;
		TYPE fy = y - v.y;

		return (fx*fx) + (fy*fy);
	}

	TYPE DistanceManhattan(const Point& v) const
	{
		return abs(v.x - x) + abs(v.y - y);
	}
};

using iPoint = Point<int>;
using fPoint = Point<float>;

#endif // __POINT_H__