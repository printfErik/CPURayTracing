#pragma once
#include "ObjFileReader.h"
#include <map>
#include "rtRay.h"

class rayTracer
{
public:
	rayTracer() {}
	bool Init(const std::string& fileName);
	bool ReadTextureFiles(const std::string& textureDir);
	bool ComputeUV();
	bool ComputeAspectRatioAndRenderPlane();
	void InitPixelArray();
	void CreatePixelIndexTo3DPointMap();
	void CreatePixelIndexToRayMap();
	void ComputePixelColor();
	rtColor RecursiveTraceRay(const rtRay& incidence, int recusiveDepth, double etai, bool isSphere, int whichObj, double lastEta);
	rtColor BlinnPhongShading(const rtMaterial& mtlColor, const rtPoint& intersection, int objIndex, const rtVector3& normal, bool isSphere, const rtPoint& newOrigin);
	void OutputFinalImage(const std::string& outFolderName);

private:

	std::unique_ptr<ObjFileReader> m_fileReader;

	rtVector3 m_u;
	rtVector3 m_v;

	std::vector<std::vector<rtColor>> m_pixels;

	rtPoint m_ul;
	rtPoint m_ur;
	rtPoint m_ll;
	rtPoint m_lr;

	std::map<rtVector2<int>, rtPoint> m_imgIndex2PointMap;
	std::map<rtVector2<int>, rtRay> m_imgIndex2RayMap;

	std::map<std::string, std::vector<rtColor>> m_textureData;
	std::map<std::string, rtVector2<int>> m_textureSize;
};