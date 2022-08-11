#include "rtPoint.h"

rtPoint rtPoint::add(const rtPoint& p, const rtVector3& v)
{
	rtPoint ans;
	ans.m_x = p.m_x + v.m_x;
	ans.m_y = p.m_y + v.m_y;
	ans.m_z = p.m_z + v.m_z;
	return ans;
}

rtVector3 rtPoint::subtract(const rtPoint& p)
{
	rtVector3 ans;
	ans.m_x = m_x - p.m_x;
	ans.m_y = m_y - p.m_y;
	ans.m_z = m_z - p.m_z;
	return ans;
}