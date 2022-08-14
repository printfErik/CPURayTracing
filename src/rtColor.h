#pragma once
class rtColor
{
public:
	rtColor() : m_r(0), m_g(0), m_b(0) {}
	rtColor(double _r, double _g, double _b) :
		m_r(_r), m_g(_g), m_b(_b) {}

	rtColor operator * (double scale);
	rtColor operator + (const rtColor& c);

	void clamp();
	int rtoi();
	int gtoi();
	int btoi();

	double m_r;
	double m_g;
	double m_b;
};