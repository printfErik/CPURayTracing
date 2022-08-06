#pragma once
#include "rtPoint.h"

class rtRay
{
public:
	rtRay();

	rtPoint m_origin;
	rtVector3 m_direction;

};