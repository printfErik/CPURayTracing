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
	float vFov = 0.f;
	rtVector2 imageSize;
	rtColor bkgColor;
	rtMaterial material;
	std::vector<rtSphere> spheres;
	std::vector<rtLight> lights;
	std::vector<rtPoint> verteices;
	std::vector<rtVector3> vertexNormals;
	std::vector<rtVector2> vertexTextureCoordinates;
	std::vector<std::vector<std::vector<int>>> faces;
	std::vector<std::string> texturePaths;
};

class ObjFileReader : public FileReaderBase
{
public:
	ObjFileReader();
	ObjFileReader(const std::string& _fileName);
	eParseRetType parseFile() override;

private:

	std::unique_ptr<ObjFileInfo> m_objFileInfo;
};