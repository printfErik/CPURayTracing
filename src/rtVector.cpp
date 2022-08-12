#include "rtVector.h"
#include <cmath>

rtVector3 rtVector3::crossProduct(const rtVector3& v1, const rtVector3& v2)
{
	rtVector3 ans;
	ans.m_x = v1.m_y * v2.m_z - v1.m_z * v2.m_y;
	ans.m_y = v1.m_z * v2.m_x - v1.m_x * v2.m_z;
	ans.m_z = v1.m_x * v2.m_y - v1.m_y * v2.m_x;

	return ans;
}

void rtVector3::twoNorm()
{
	float div = std::sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
	m_x = m_x / div;
	m_y = m_y / div;
	m_z = m_z / div;
}

rtVector3 rtVector3::getTwoNorm()
{
	rtVector3 ans;
	float div = std::sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
	ans.m_x = m_x / div;
	ans.m_y = m_y / div;
	ans.m_z = m_z / div;
	return ans;
}

void rtVector3::selfScale(float s)
{
	m_x *= s;
	m_y *= s;
	m_z *= s;
}

rtVector3 rtVector3::scale(float s)
{
	rtVector3 ans;
	ans.m_x = m_x * s;
	ans.m_y = m_y * s;
	ans.m_z = m_z * s;
	return ans;
}

rtVector3 rtVector3::add(const rtVector3& v)
{
	rtVector3 ans;
	ans.m_x = m_x + v.m_x;
	ans.m_y = m_y + v.m_y;
	ans.m_z = m_z + v.m_z;
	return ans;
}

rtVector3 rtVector3::subtract(const rtVector3& v)
{
	rtVector3 ans;
	ans.m_x = m_x - v.m_x;
	ans.m_y = m_y - v.m_y;
	ans.m_z = m_z - v.m_z;
	return ans;
}

float rtVector3::dotProduct(const rtVector3& v1, const rtVector3& v2)
{
	return v1.m_x * v2.m_x + v1.m_y * v2.m_y + v1.m_z * v2.m_z;
}

float rtVector3::area(const rtVector3& v1, const rtVector3& v2)
{
	float x = v1.m_y * v2.m_z - v1.m_z * v2.m_y;
	float y = v1.m_z * v2.m_x - v1.m_x * v2.m_z;
	float z = v1.m_x * v2.m_y - v1.m_y * v2.m_x;
	return std::sqrt(x * x + y * y + z * z);
}

float rtVector3::length()
{
	return std::sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
}

