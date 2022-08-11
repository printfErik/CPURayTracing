#pragma once
#include "rtVector.h"

class rtPoint
{
public:
	rtPoint()
		: m_x(0), m_y(0), m_z(0) {}
	rtPoint(float _x, float _y, float _z)
		: m_x(_x), m_y(_y), m_z(_z) {}

	static rtPoint add(const rtPoint& p, const rtVector3& v);
	rtVector3 subtract(const rtPoint& v);

	float m_x;
	float m_y;
	float m_z;				
};