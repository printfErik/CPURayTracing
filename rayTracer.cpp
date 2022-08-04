#include "rayTracer.h"
#include <iostream>

#include <cmath>
#include <corecrt_math_defines.h>

bool rayTracer::Init(const std::string& fileName)
{
	m_fileReader = std::make_unique<ObjFileReader>(fileName);

	if (eParseRetType::kSuccess != m_fileReader->parseFile())
	{
		return false;
	}
	return true;
}

bool rayTracer::ComputeUV()
{
	auto fileInfo = m_fileReader->getFileInfo();

	// check viewDir and upDir are not parallel
	m_u = rtVector3::crossProduct(fileInfo->upDir, fileInfo->viewDir);

	if (m_u.m_x == 0.f && m_u.m_y == 0.f && m_u.m_z == 0.f)
	{
		std::cout << "viewDir and upDir are parallel" << std::endl;
		return false;
	}

	m_u.twoNorm();
	m_v = rtVector3::crossProduct(m_u, fileInfo->viewDir);
	m_v.twoNorm();

	return true;
}

bool rayTracer::ComputeAspectRatioAndRenderPlane()
{
	auto fileInfo = m_fileReader->getFileInfo();

	float aspectRatio = fileInfo->imageSize.m_x / fileInfo->imageSize.m_y;
	float distance = 5.f; // random number
	float height = 2.f * distance * std::tan(fileInfo->vFov * M_PI / 360.f);
	float width = height * aspectRatio;
	
	rtVector3 n = fileInfo->viewDir;
	n.twoNorm();

	rtPoint center = rtPoint::add(fileInfo->eye, n.scale(distance));
	m_ul = rtPoint::add(rtPoint::add(center, m_u.scale(-width / 2.f)), m_v.scale(height / 2.f));
	m_ur = rtPoint::add(fileInfo->eye, n.scale(distance).add(m_u.scale(width / 2.f).add(m_v.scale(height / 2.f))));
	m_ll = rtPoint::add(fileInfo->eye, n.scale(distance).add(m_u.scale(-width / 2.f).add(m_v.scale(-height / 2.f))));
	m_lr = rtPoint::add(fileInfo->eye, n.scale(distance).add(m_u.scale(width / 2.f).add(m_v.scale(-height / 2.f))));

	return true;
}

void rayTracer::InitPixelArray()
{
	auto fileInfo = m_fileReader->getFileInfo();

	m_pixels.resize(fileInfo->imageSize.m_x, std::vector<rtColor>(fileInfo->imageSize.m_y, fileInfo->bkgColor));

	for (int i = 0; i < fileInfo->imageSize.m_x; i++)
	{
		for (int j = 0; j < fileInfo->imageSize.m_y; j++)
		{
			m_pixels[i][j] = fileInfo->bkgColor;
		}
	}
}

void rayTracer::CreatePixelIndexTo3DPointMap()
{
	auto fileInfo = m_fileReader->getFileInfo();
	rtVector3 d_h = m_ur.subtract(m_ul).scale(1.f / fileInfo->imageSize.m_x);
	rtVector3 d_v = m_ll.subtract(m_ul).scale(1.f / fileInfo->imageSize.m_y);

	rtVector3 d_ch = m_ur.subtract(m_ul).scale(1.f / (2.f * fileInfo->imageSize.m_x));
	rtVector3 d_cv = m_ll.subtract(m_ul).scale(1.f / (2.f * fileInfo->imageSize.m_y));
	
	for (int i = 0; i < fileInfo->imageSize.m_x; i++)
	{
		for (int j = 0; j < fileInfo->imageSize.m_y; j++)
		{
			rtPoint p = rtPoint::add(m_ul, d_h.scale((float)i).add(d_v.scale((float)j)).add(d_ch).add(d_cv));
			rtVector2 index(i, j);
			m_imgIndex2PointMap[index] = p;
		}
	}

}