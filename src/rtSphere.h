#pragma once
#include "rtVector.h"
#include "rtPoint.h"
class rtSphere
{
public:
	rtSphere() {}
	rtSphere(rtPoint& _center, double radius)
		: m_center(_center), m_radius(radius) {}

	void setCenter(double x, double y, double z);


	int m_materialIndex = -1;


	rtPoint m_center;
	double m_radius = 0.0;
	
};