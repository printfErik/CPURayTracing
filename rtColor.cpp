#include "rtColor.h"

rtColor rtColor::operator* (float scale)
{
	rtColor ans;
	ans.m_r = m_r * scale;
	ans.m_g = m_g * scale;
	ans.m_b = m_b * scale;
	return ans;
}

rtColor rtColor::operator + (const rtColor& c)
{
	rtColor ans;
	ans.m_r = m_r + c.m_r;
	ans.m_g = m_g + c.m_g;
	ans.m_b = m_b + c.m_b;
	return ans;
}

void rtColor::clamp()
{
	m_r = m_r > 1 ? 1.f : m_r;
	m_g = m_g > 1 ? 1.f : m_g;
	m_b = m_b > 1 ? 1.f : m_b;
}

int rtColor::rtoi()
{
	return m_r > 1 ? 255 : static_cast<int>(m_r * 255);
}

int rtColor::gtoi()
{
	return m_g > 1 ? 255 : static_cast<int>(m_g * 255);
}

int rtColor::btoi()
{
	return m_b > 1 ? 255 : static_cast<int>(m_b * 255);
}