#pragma once
#include "rtPoint.h"
#include "rtColor.h"
#include "rtVector.h"

enum class eLightType
{
	kDirectionalLight,
	kPointLight,
	kSpotlight,
	kAttPointLight,
	kAttSpotlight,
	kUndefined
};

class rtLight
{
public:
	rtLight() {}
	rtLight(eLightType _type, const rtPoint& _center, const rtColor& _color, const rtVector3& _vec3)
		: m_type(_type), m_center(_center), m_color(_color), m_vec3(_vec3) {}

	void setType(eLightType _type)
	{
		m_type = _type;
	}

	void setCenter(const rtPoint& _p)
	{
		m_center.m_x = _p.m_x;
		m_center.m_y = _p.m_y;
		m_center.m_z = _p.m_z;
	}

	void setColor(const rtColor& _c)
	{
		m_color.m_r = _c.m_r;
		m_color.m_g = _c.m_g;
		m_color.m_b = _c.m_b;
	}

	void setVector3(const rtVector3& v)
	{
		m_vec3.m_x = v.m_x;
		m_vec3.m_y = v.m_y;
		m_vec3.m_z = v.m_z;
	}

	eLightType getType()
	{
		return m_type;
	}

	void setTheta(double t)
	{
		m_theta = t;
	}

	void setAttAttribute(double c1, double c2, double c3);

	eLightType m_type = eLightType::kUndefined;
	rtPoint m_center;
	rtColor m_color;

	rtVector3 m_vec3;

	

	double m_theta = 0.0;
	double m_c1 = 0.0;
	double m_c2 = 0.0;
	double m_c3 = 0.0;
};