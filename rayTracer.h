#pragma once
#include "ObjFileReader.h"

class rayTracer
{
public:
	rayTracer() {}
	bool Init(const std::string& fileName);
	bool ComputeUV();
	bool ComputeAspectRatioAndRenderPlane();

private:

	std::unique_ptr<ObjFileReader> m_fileReader;

	rtVector3 m_u;
	rtVector3 m_v;
};