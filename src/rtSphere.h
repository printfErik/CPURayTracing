#pragma once
#include "rtVector.h"
#include "rtPoint.h"
class rtSphere
{
public:
	rtSphere() {}
	rtSphere(rtPoint& _center, float radius)
		: m_center(_center), m_radius(radius) {}

	void setCenter(float x, float y, float z);


	int m_materialIndex = -1;


	rtPoint m_center;
	float m_radius = 0.f;
	
};