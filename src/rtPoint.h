#pragma once
#include "rtVector.h"

class rtPoint
{
public:
	rtPoint()
		: m_x(0), m_y(0), m_z(0) {}
	rtPoint(double _x, double _y, double _z)
		: m_x(_x), m_y(_y), m_z(_z) {}

	static rtPoint add(const rtPoint& p, const rtVector3& v);
	rtVector3 subtract(const rtPoint& v) const;

	double m_x;
	double m_y;
	double m_z;				
};