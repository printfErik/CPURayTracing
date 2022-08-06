#include "rayTracer.h"
#include <iostream>

#include <cmath>
#include <corecrt_math_defines.h>

static constexpr int MAX_RECURSIVE_DEPTH = 7;

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

void rayTracer::CreatePixelIndexToRayMap()
{
	auto fileInfo = m_fileReader->getFileInfo();
	for (int i = 0; i < fileInfo->imageSize.m_x; i++)
	{
		for (int j = 0; j < fileInfo->imageSize.m_y; j++)
		{
			rtVector2 index(i, j);
			rtPoint end = m_imgIndex2PointMap[index];
			rtVector3 rayDir = end.subtract(fileInfo->eye);
			rayDir.twoNorm();
			rtRay ray;
			ray.m_origin = fileInfo->eye;
			ray.m_direction = rayDir;
			m_imgIndex2RayMap[index] = ray;
		}
	}
}

void rayTracer::ComputePixelColor()
{
	auto fileInfo = m_fileReader->getFileInfo();
	for (int i = 0; i < fileInfo->imageSize.m_x; i++)
	{
		for (int j = 0; j < fileInfo->imageSize.m_y; j++)
		{
			rtVector2 index(i, j);
			rtRay ray = m_imgIndex2RayMap[index];
			rtColor pixelColor = RecursiveTraceRay(ray, )
		}
	}
}

rtColor rayTracer::RecursiveTraceRay(rtRay& incidence, int recusiveDepth, double etai, bool isSphere, int objIndex, double lastEta)
{
	auto fileInfo = m_fileReader->getFileInfo();
	if (recusiveDepth == MAX_RECURSIVE_DEPTH)
	{
		float r, g, b;
		if (isSphere)
		{
			r = fileInfo->materials[fileInfo->spheres[objIndex].m_materialIndex].m_odr;
			g = fileInfo->materials[fileInfo->spheres[objIndex].m_materialIndex].m_odg;
			b = fileInfo->materials[fileInfo->spheres[objIndex].m_materialIndex].m_odb;
		}
		else
		{
			r = fileInfo->materials[fileInfo->faceMaterialIndexs[objIndex]].m_odr;
			g = fileInfo->materials[fileInfo->faceMaterialIndexs[objIndex]].m_odg;
			b = fileInfo->materials[fileInfo->faceMaterialIndexs[objIndex]].m_odb;
		}
		return rtColor(r, g, b);
	}

	// determine is a ray intersects with an object;
	bool exit = false;
	rtColor hit;
	float t1 = std::numeric_limits<float>::infinity();

	std::vector<rtVector3> tris;

	std::vector<float> alphas, betas, gammas;

	bool isSphere_ = true;
	int objIndex_ = -1;
	
	int total = 0;

	for (int sphereIndex = 0; sphereIndex < fileInfo->spheres.size(); sphereIndex++)
	{
		float xc = fileInfo->spheres[sphereIndex].m_center.m_x;
		float yc = fileInfo->spheres[sphereIndex].m_center.m_y;
		float zc = fileInfo->spheres[sphereIndex].m_center.m_z;
		float r = fileInfo->spheres[sphereIndex].m_radius;

		float distanceX = incidence.m_origin.m_x - xc;
		float distanceY = incidence.m_origin.m_y - yc;
		float distanceZ = incidence.m_origin.m_z - zc;

		float A = 1.f;
		float B = 2.f * (incidence.m_direction.m_x * distanceX +
						 incidence.m_direction.m_y * distanceY +
						 incidence.m_direction.m_z * distanceZ);
		float C = distanceX * distanceX + distanceY * distanceY + distanceZ * distanceZ - r * r;

		float delta = B * B - 4.f * A * C;

		if // (delta close to zero)
		{

		}
	}
}