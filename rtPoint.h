#pragma once
class rtPoint
{
public:
	rtPoint()
		: m_x(0), m_y(0), m_z(0) {}
	rtPoint(float _x, float _y, float _z)
		: m_x(_x), m_y(_y), m_z(_z) {}

	float m_x;
	float m_y;
	float m_z;
};