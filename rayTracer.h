#pragma once
#include "ObjFileReader.h"
#include <map>

class rayTracer
{
public:
	rayTracer() {}
	bool Init(const std::string& fileName);
	bool ComputeUV();
	bool ComputeAspectRatioAndRenderPlane();
	void InitPixelArray();
	void CreatePixelIndexTo3DPointMap();

private:

	std::unique_ptr<ObjFileReader> m_fileReader;

	rtVector3 m_u;
	rtVector3 m_v;

	std::vector<std::vector<rtColor>> m_pixels;

	rtPoint m_ul;
	rtPoint m_ur;
	rtPoint m_ll;
	rtPoint m_lr;

	std::map<rtVector2, rtPoint> m_imgIndex2PointMap;
};