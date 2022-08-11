#pragma once
#include "ObjFileReader.h"
#include <map>
#include "rtRay.h"

class rayTracer
{
public:
	rayTracer() {}
	bool Init(const std::string& fileName);
	bool ReadTextureFiles();
	bool ComputeUV();
	bool ComputeAspectRatioAndRenderPlane();
	void InitPixelArray();
	void CreatePixelIndexTo3DPointMap();
	void CreatePixelIndexToRayMap();
	void ComputePixelColor();
	rtColor RecursiveTraceRay(rtRay& incidence, int recusiveDepth, float etai, bool isSphere, int whichObj, float lastEta);
	rtColor BlinnPhongShading(rtMaterial& mtlColor, rtPoint& intersection, int objIndex, rtVector3& normal, bool isSphere, rtPoint& newOrigin);
	void OutputFinalImage();

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
	std::map<rtVector2, rtRay> m_imgIndex2RayMap;

	std::map<std::string, std::vector<rtColor>> m_textureData;
	std::map<std::string, rtVector2> m_textureSize;
};