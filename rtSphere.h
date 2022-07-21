#pragma once
#include "rtVector.h"
#include "rtPoint.h"
class rtSphere
{
public:

	rtSphere() = default;
	rtSphere(rtPoint& _center, float radius)
		: m_center(_center), m_radius(radius) {}

private:
	
	rtPoint m_center;
	float m_radius;
};