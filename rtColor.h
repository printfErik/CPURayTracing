#pragma once
class rtColor
{
public:
	rtColor() : m_r(0), m_g(0), m_b(0) {}
	rtColor(float _r, float _g, float _b) :
		m_r(_r), m_g(_g), m_b(_b) {}

	rtColor operator * (float scale);
	rtColor operator + (const rtColor& c);
	float m_r;
	float m_g;
	float m_b;
};