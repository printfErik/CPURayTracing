#pragma once
#include "rtVector.h"
#include "rtPoint.h"
class rtSphere
{
public:
	rtSphere() {}
	rtSphere(rtPoint& _center, float radius)
		: m_center(_center), m_radius(radius) {}

	void setCenter(float x, float y, float z)
	{
		m_center.m_x = x;
		m_center.m_y = y;
		m_center.m_z = z;
	}

private:
	rtPoint m_center;
	float m_radius = 0.f;
};