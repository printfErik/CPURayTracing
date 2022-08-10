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