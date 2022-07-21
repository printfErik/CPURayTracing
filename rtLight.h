#pragma once
#include "rtPoint.h"
#include "rtColor.h"
#include "rtVector.h"

enum class eLightType
{
	kPointLight
};

class rtLight
{
public:
	rtLight(eLightType _type, const rtPoint& _center, const rtColor& _color, const rtVetcor3& _vec3)
		: m_type(_type), m_center(_center), m_color(_color), m_vec3(_vec3) {}

private:
	rtPoint m_center;
	rtColor m_color;

	rtVector3 m_vec3;

	eLightType m_type;
};