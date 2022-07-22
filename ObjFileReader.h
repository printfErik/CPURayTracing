#pragma once
#include <vector>
#include <string>
#include <memory>
#include "rtVector.h"
#include "rtColor.h"
#include "rtMaterial.h"
#include "rtLight.h"
#include "rtSphere.h"
#include "FileReader.h"

using ObjKeywords = std::string;

struct ObjFileInfo
{
	rtVector3 eye;
	rtVector3 viewDir;
	rtVector3 upDir;
	float vFov;
	rtVector2 imageSize;
	rtColor bkgColor;
	rtMaterial material;
	std::vector<rtSphere> spheres;
	std::vector<rtLight> lights;
	std::vector<rtPoint> verteices;
	std::vector<rtVector3> vertexNormals;
	std::vector<rtVector2> vertexTextureCoordinates;
	std::vector<std::vector<std::vector<int>>> faces;
	std::string texturePath;
};

class ObjFileReader : public FileReaderBase
{
public:
	ObjFileReader();

	eParseRetType parseFile() override;

private:

	std::unique_ptr<ObjFileInfo> m_objFileInfo;
};