#include "rayTracer.h"
#include "PpmFileReader.h"
#include <iostream>

#include <cmath>
#include <corecrt_math_defines.h>

static constexpr int MAX_RECURSIVE_DEPTH = 7;
static constexpr float EPSILON = 0.00000005f;

bool rayTracer::Init(const std::string& fileName)
{
	m_fileReader = std::make_unique<ObjFileReader>(fileName);

	if (eParseRetType::kSuccess != m_fileReader->parseFile())
	{
		return false;
	}
	return true;
}

bool rayTracer::ReadTextureFiles()
{
	auto fileInfo = m_fileReader->getFileInfo();
	for (int i = 0; i < fileInfo->materials.size(); i++)
	{
		std::string texName = fileInfo->materials[i].getTextureFile();
		if (!texName.empty())
		{
			std::unique_ptr<ppmFileReader> ppmFileReaderInstance = std::make_unique<ppmFileReader>(texName);
			std::vector<rtColor> texture;
			rtVector2 size;
			ppmFileReaderInstance->getTextureArray(texture, size);
			m_textureData[texName] = texture;
			m_textureSize[texName] = size;
		}
	}
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
	float height = 2.f * distance * std::tan(fileInfo->vFov * static_cast<float>(M_PI) / 360.f);
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

	m_pixels.resize(static_cast<int>(fileInfo->imageSize.m_x), std::vector<rtColor>(static_cast<int>(fileInfo->imageSize.m_y), fileInfo->bkgColor));

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
			rtColor pixelColor = RecursiveTraceRay(ray, 0, 1.f, true, -1, 1.f);
			pixelColor.clamp();
			m_pixels[i][j] = pixelColor;
		}
	}
}

rtColor rayTracer::RecursiveTraceRay(rtRay& incidence, int recusiveDepth, float etai, bool isSphere, int objIndex, float lastEta)
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

	std::vector<rtVector3> triNormals;

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

		if (delta <= EPSILON && delta >= -EPSILON)// (delta close to zero)
		{
			float tempT = (-B) / (2.f * A);
			if (tempT < t1 && tempT > EPSILON)
			{
				t1 = tempT;
				objIndex = sphereIndex;
			}
			else 
			{
				total++;
			}
		}
		else if (delta > EPSILON)
		{
			float tempT1 = (std::sqrt(delta) - B) / (2.f * A);
			float tempT2 = (-std::sqrt(delta) - B) / (2.f * A);
			bool isHit = false;
			if ((tempT1 < t1) && tempT1 > EPSILON)
			{
				t1 = tempT1;
				objIndex = sphereIndex;
				isHit = true;
			}

			if ((tempT2 < t1) && tempT2 > EPSILON)
			{
				t1 = tempT2;
				objIndex = sphereIndex;
				isHit = true;
			}

			if (isHit)
			{
				total++;
			}
		}
		else
		{
			total++;
		}
	}

	// check for all triangles
	for (int triangleIndex = 0; triangleIndex < fileInfo->faces.size(); triangleIndex++)
	{
		rtPoint firstVertex = fileInfo->verteices[fileInfo->faces[triangleIndex][0][0] - 1];
		rtPoint secondVertex = fileInfo->verteices[fileInfo->faces[triangleIndex][1][0] - 1];;
		rtPoint thirdVertex = fileInfo->verteices[fileInfo->faces[triangleIndex][2][0] - 1];;
		rtVector3 e1 = secondVertex.subtract(firstVertex);
		rtVector3 e2 = thirdVertex.subtract(firstVertex);
		rtVector3 normal = rtVector3::crossProduct(e1, e2);
		float A = normal.m_x;
		float B = normal.m_y;
		float C = normal.m_z;
		float D = -(firstVertex.m_x * A + firstVertex.m_y * B + firstVertex.m_z * C);
		float deterimint = A * incidence.m_direction.m_x + B * incidence.m_direction.m_y + C * incidence.m_direction.m_z;

		// check if viewdir is parallel to the surface
		if (deterimint < EPSILON && deterimint > -EPSILON)
		{
			total++;
			continue;
		}
		float tTri = -(A * incidence.m_origin.m_x + B * incidence.m_origin.m_y + C * incidence.m_origin.m_z + D) / deterimint;
		if (tTri < 0.f || tTri > t1)
		{
			total++;
			continue;
		}
		rtPoint hitPoint = rtPoint::add(incidence.m_origin, incidence.m_direction.scale(tTri));
		rtVector3 e3 = hitPoint.subtract(secondVertex);
		rtVector3 e4 = hitPoint.subtract(thirdVertex);
		float totalArea = rtVector3::area(e1, e2);
		float aArea = rtVector3::area(e3, e4);
		float bArea = rtVector3::area(e4, e2);
		float cArea = rtVector3::area(e1, e3);
		float alpha = aArea / totalArea;
		float beta = bArea / totalArea;
		float gamma = cArea / totalArea;

		// determine Barycentric coordinates
		if (alpha < 1 && alpha > 0 && beta < 1 && beta > 0 && gamma > 0 && gamma < 1 && alpha + beta + gamma - 1 < EPSILON)
		{
			t1 = tTri;
			isSphere = false;
			objIndex = triangleIndex;
			alphas.push_back(alpha);
			betas.push_back(beta);
			gammas.push_back(gamma);
			if (fileInfo->faces[triangleIndex][0][2] == 0)
			{
				// if no vn, which means flat shading
				triNormals.push_back(normal);
			}
			else
			{
				// smooth shading
				rtVector3 first_nromal = fileInfo->vertexNormals[fileInfo->faces[triangleIndex][0][2] - 1];
				rtVector3 second_nromal = fileInfo->vertexNormals[fileInfo->faces[triangleIndex][1][2] - 1];
				rtVector3 third_nromal = fileInfo->vertexNormals[fileInfo->faces[triangleIndex][2][2] - 1];
				rtVector3 smooth_normal = (first_nromal.scale(alpha).add(second_nromal.scale(beta)).add(third_nromal.scale(gamma))).getTwoNorm();
				triNormals.push_back(smooth_normal);
			}
		}
		else
		{
			total++;
			continue;
		}
	}
	// if we detect an intersection,find this point and change img color
	if (total != fileInfo->spheres.size() + fileInfo->faces.size())
	{
		rtVector3 I = incidence.m_direction.getTwoNorm().scale(-1);
		rtVector3 rayDir = incidence.m_direction.scale(t1);
		rtPoint closest = rtPoint::add(incidence.m_origin, rayDir);
		rtMaterial temp;
		rtVector3 normal;
		if (isSphere)
		{
			// compute normal vector for sphere which will be used in phong equation
			temp = fileInfo->materials[fileInfo->spheres[objIndex].m_materialIndex];
			rtPoint sphereCenter(fileInfo->spheres[objIndex].m_center.m_x, fileInfo->spheres[objIndex].m_center.m_y, fileInfo->spheres[objIndex].m_center.m_z);
			normal = closest.subtract(sphereCenter).getTwoNorm();
		}
		else
		{
			// compute normal vector for triangle which will be used in phong equation
			temp = fileInfo->materials[fileInfo->faceMaterialIndexs[objIndex]];
			normal = triNormals.back().getTwoNorm();
		}

		if (rtVector3::dotProduct(I, normal) < 0)
		{
			normal = normal.scale(-1);
		}

		if (isSphere && (isSphere == isSphere_) && (objIndex == objIndex_))
		{
			exit = true;
		}

		std::string name = temp.getTextureFile();
		if (!name.empty()) // if texture detected
		{
			rtColor texelColor;
			float textureU, textureV;
			if (!isSphere)
			{
				//mapping texture to a triangle
				rtVector2 first_2d = fileInfo->vertexTextureCoordinates[fileInfo->faces[objIndex][0][1] - 1];
				rtVector2 second_2d = fileInfo->vertexTextureCoordinates[fileInfo->faces[objIndex][1][1] - 1];
				rtVector2 third_2d = fileInfo->vertexTextureCoordinates[fileInfo->faces[objIndex][2][1] - 1];
				// using Barycentric coordinates
				textureU = (alphas.back() * first_2d.m_x + betas.back() * second_2d.m_x + gammas.back() * third_2d.m_x);
				textureV = (alphas.back() * first_2d.m_y + betas.back() * second_2d.m_y + gammas.back() * third_2d.m_y);
			}
			else
			{
				//mapping texture to a sphere
				float phi = std::acos((closest.m_z - fileInfo->spheres[objIndex].m_center.m_z) / fileInfo->spheres[objIndex].m_radius);
				float zeta = std::atan2((closest.m_y - fileInfo->spheres[objIndex].m_center.m_y), (closest.m_x - fileInfo->spheres[objIndex].m_center.m_x));
				float textureU, textureV;
				textureV = phi / M_PI;
				textureU = (zeta + M_PI) / (2 * M_PI);
			}
			texelColor = m_textureData[name][static_cast<int>(textureV * (m_textureSize[name].m_y - 1.f) + 0.5f) * static_cast<int>(m_textureSize[name].m_x)
										   + static_cast<int>(textureU * (m_textureSize[name].m_x - 1.f) + 0.5f)];
			rtMaterial tempMtl(texelColor.m_r / 255.f, texelColor.m_g / 255.f, texelColor.m_b / 255.f,
								temp.m_osr, temp.m_osg, temp.m_osb,
								temp.m_ka, temp.m_kd, temp.m_ks, temp.m_falloff, temp.m_alpha, temp.m_eta);
			hit = BlinnPhongShading(tempMtl, closest, objIndex, normal, isSphere, incidence.m_origin);
		}
		else // no texture detected, apply normal phong equation
		{
			hit = BlinnPhongShading(temp, closest, objIndex, normal, isSphere, incidence.m_origin);
		}

		rtVector3 forwardEpsilon = incidence.m_direction.scale(t1 + EPSILON);
		rtPoint forward = rtPoint::add(incidence.m_origin, forwardEpsilon);

		rtVector3 backwardEpsilon = incidence.m_direction.scale(t1 - EPSILON);
		rtPoint backward = rtPoint::add(incidence.m_origin, backwardEpsilon);

		rtRay reflection;
		rtVector3 reflectionDir;

		float cosphii = std::abs(rtVector3::dotProduct(I, normal));
		float sinphii = std::pow(1.f - cosphii * cosphii, 0.5f);
		reflectionDir = normal.scale(cosphii * 2.f).add(incidence.m_direction.getTwoNorm());
		reflection.m_origin = backward;
		reflection.m_direction = reflectionDir;

		float curEta = temp.m_eta;
		float curAlpha = temp.m_alpha;

		if (exit)
		{
			curEta = lastEta;
		}
		float F0 = ((curEta - etai) / (curEta + etai)) * ((curEta - etai) / (curEta + etai));
		float FresnelReflectance = F0 + (1.f - F0) * std::pow((1.f - cosphii), 5.f);

		rtRay transmission;
		rtVector3 transmissionDir;
		if (!isSphere)
		{
			transmissionDir = incidence.m_direction.getTwoNorm();
		}
		else
		{
			float firstCoff = std::pow(1.f - (etai / curEta) * (etai / curEta) * (1.f - cosphii * cosphii), 0.5f);
			rtVector3 transmissionDir = normal.scale(-1.f).scale(firstCoff).add(normal.scale(cosphii * (etai / curEta))).add(I.scale(-1.f).scale(etai / curEta));
			transmissionDir.twoNorm();
		}

		transmission.m_origin = forward;
		transmission.m_direction = transmissionDir;

		if (sinphii > (curEta / etai))
		{
			hit = hit + (RecursiveTraceRay(reflection, recusiveDepth + 1, etai, isSphere, objIndex, etai) * FresnelReflectance);
		}
		else
		{
			rtColor trans;
			trans = RecursiveTraceRay(transmission, recusiveDepth + 1, isSphere ? curEta : etai, isSphere, objIndex, etai) * ((1.f - FresnelReflectance) * (1.f - curAlpha));
			hit = hit + (RecursiveTraceRay(reflection, recusiveDepth + 1, etai, isSphere, objIndex, etai) * FresnelReflectance) + trans;
		}
	}
	else
	{
		return fileInfo->bkgColor;
	}

	return hit;
}

rtColor rayTracer::BlinnPhongShading(rtMaterial& mtlColor, rtPoint& intersection, int objIndex, rtVector3& normal, bool isSphere, rtPoint& newOrigin)
{
	auto fileInfo = m_fileReader->getFileInfo();
	rtPoint center;

	// set intial color based on material property
	float r = mtlColor.m_ka * mtlColor.m_odr;
	float g = mtlColor.m_ka * mtlColor.m_odg;
	float b = mtlColor.m_ka * mtlColor.m_odb;

	// shoot lights to the intersection
	for (int i = 0; i < fileInfo->lights.size(); i++)
	{
		auto curLight = fileInfo->lights[i];
		float shadowMask = 1.f;
		rtVector3 lightDir;
		float maxT1;
		float fatt = 1;

		// calculate the L vector in phong equation
		switch (curLight.getType())
		{
		case eLightType::kAttPointLight:
		case eLightType::kPointLight:
		{
			rtPoint lightSource(curLight.m_center.m_x, curLight.m_center.m_y, curLight.m_center.m_z);
			lightDir = lightSource.subtract(intersection);
			maxT1 = lightDir.length();

			// for point light, use fatt to indicate "Light Source Attenuation"
			fatt = 1.f / (curLight.m_c1 + curLight.m_c2 * maxT1 + curLight.m_c3 * maxT1 * maxT1);
		}
		break;
		case eLightType::kDirectionalLight:
		{
			lightDir = rtVector3(-curLight.m_center.m_x, -curLight.m_center.m_y, -curLight.m_center.m_z);
			lightDir.twoNorm();
		}
		break;
		case eLightType::kSpotlight:
		case eLightType::kAttSpotlight:
		{
			rtVector3 vobj = intersection.subtract(curLight.m_center).getTwoNorm();
			rtVector3 vlight(curLight.m_vec3.m_x, curLight.m_vec3.m_y, curLight.m_vec3.m_z);
			vlight.twoNorm();

			// check if this spotlight is valid for the single intersection 
			// then add them to the global lights vector and count the number 
			if (rtVector3::dotProduct(vlight, vobj) >= std::cos(curLight.m_theta * M_PI / 180.f))
			{
				rtPoint lightSource(curLight.m_center.m_x, curLight.m_center.m_y, curLight.m_center.m_z);
				lightDir = lightSource.subtract(intersection);
				maxT1 = lightDir.length();

				// for point light, use fatt to indicate "Light Source Attenuation"
				fatt = 1.f / (curLight.m_c1 + curLight.m_c2 * maxT1 + curLight.m_c3 * maxT1 * maxT1);
			}
			else
			{
				continue;
			}
		}
		break;
		}

		// calculate the V and H vectors in phong equation
		rtVector3 V = newOrigin.subtract(intersection).getTwoNorm();
		rtVector3 H = lightDir.add(V).getTwoNorm();
		float nl = rtVector3::dotProduct(normal, lightDir);
		float nh = rtVector3::dotProduct(normal, H);

		// shoot shadow rays to check shadow
		float t1 = std::numeric_limits<float>::infinity();
		rtRay shadowRay;
		shadowRay.m_origin = intersection;
		shadowRay.m_direction = lightDir;
		float x0 = shadowRay.m_origin.m_x;
		float y0 = shadowRay.m_origin.m_y;
		float z0 = shadowRay.m_origin.m_z;
		float xd = shadowRay.m_direction.m_x;
		float yd = shadowRay.m_direction.m_y;
		float zd = shadowRay.m_direction.m_z;

		for (int k = 0; k < fileInfo->spheres.size(); k++)
		{
			if (k == objIndex && isSphere)
			{
				continue;
			}

			float xc = fileInfo->spheres[k].m_center.m_x;
			float yc = fileInfo->spheres[k].m_center.m_y;
			float zc = fileInfo->spheres[k].m_center.m_z;
			float r = fileInfo->spheres[k].m_radius;

			float A = 1.f;
			float B = 2.f * (xd * (x0 - xc) + yd * (y0 - yc) + zd * (z0 - zc));
			float C = (x0 - xc) * (x0 - xc) + (y0 - yc) * (y0 - yc) + (z0 - zc) * (z0 - zc) - r * r;
			float delta = B * B - 4.f * A * C;

			// check discriminant
			if (delta <= EPSILON && delta >= -EPSILON)
			{
				float temp_t1 = (-B) / (2.f * A);
				if (temp_t1 > 0 && temp_t1 < t1)
				{
					t1 = temp_t1;
				}
			}
			else if (delta > EPSILON)
			{
				float temp_t1 = (std::sqrt(delta) - B) / (2.f * A);
				float temp_t2 = (-std::sqrt(delta) - B) / (2.f * A);

				float comparator = curLight.getType() == eLightType::kDirectionalLight ? std::numeric_limits<float>::infinity() : maxT1;
				if ((temp_t1 > 0 && temp_t1 != comparator) || (temp_t2 > 0 && temp_t2 != comparator))
				{
					shadowMask = shadowMask * (1.f - fileInfo->materials[fileInfo->spheres[k].m_materialIndex].m_alpha);
				}

				if ((temp_t1 < t1) && temp_t1 > 0)
				{
					t1 = temp_t1;
				}

				if ((temp_t2 < t1) && temp_t2 > 0)
				{
					t1 = temp_t2;
				}
			}
		}
		// shoot shadow rays and check if it intersect with triangles
		for (int triIndex = 0; triIndex < fileInfo->faces.size(); triIndex++)
		{
			if (triIndex == objIndex && (!isSphere))
			{
				continue;
			}
			// same logic when shooting viewing ray
			rtPoint firstVertex = fileInfo->verteices[fileInfo->faces[triIndex][0][0] - 1];
			rtPoint secondVertex = fileInfo->verteices[fileInfo->faces[triIndex][1][0] - 1];;
			rtPoint thirdVertex = fileInfo->verteices[fileInfo->faces[triIndex][2][0] - 1];;
			rtVector3 e1 = secondVertex.subtract(firstVertex);
			rtVector3 e2 = thirdVertex.subtract(firstVertex);
			rtVector3 normal = rtVector3::crossProduct(e1, e2);
			float A = normal.m_x;
			float B = normal.m_y;
			float C = normal.m_z;
			float D = -(firstVertex.m_x * A + firstVertex.m_y * B + firstVertex.m_z * C);
			float deterimint = A * xd + B * yd + C * zd;
			if (deterimint <= EPSILON && deterimint >= -EPSILON)
			{
				continue;
			}
			float triT = -(A * x0 + B * y0 + C * z0 + D) / deterimint;
			if (triT < 0)
			{
				continue;
			}
			rtPoint p_for_tri = rtPoint::add(intersection, shadowRay.m_direction.scale(triT));
			rtVector3 e3 = p_for_tri.subtract(secondVertex);
			rtVector3 e4 = p_for_tri.subtract(thirdVertex);
			float totalArea = rtVector3::area(e1, e2);
			float aArea = rtVector3::area(e3, e4);
			float bArea = rtVector3::area(e4, e2);
			float cArea = rtVector3::area(e1, e3);
			float alpha = aArea / totalArea;
			float beta = bArea / totalArea;
			float gamma = cArea / totalArea;
			if (alpha < 1.f && alpha > 0.f && beta < 1.f && beta > 0.f && gamma > 0.f && gamma < 1.f && alpha + beta + gamma - 1 < EPSILON)
			{
				float comparator = curLight.getType() == eLightType::kDirectionalLight ? std::numeric_limits<float>::infinity() : maxT1;

				if (triT > 0 && triT < comparator)
				{
					shadowMask = shadowMask * (1.f - fileInfo->materials[fileInfo->faceMaterialIndexs[triIndex]].m_alpha);
				}

				if (triT < t1 && triT > 0) // todo might not correct
				{
					t1 = triT;
				}
			}
		}

		// using phong equation to calculate rgb values
		r += shadowMask * fatt * (mtlColor.m_kd * mtlColor.m_odr * std::max(nl, 0.f) + mtlColor.m_ks * mtlColor.m_osr * std::pow(std::max(nh, 0.f), mtlColor.m_falloff));
		g += shadowMask * fatt * (mtlColor.m_kd * mtlColor.m_odg * std::max(nl, 0.f) + mtlColor.m_ks * mtlColor.m_osg * std::pow(std::max(nh, 0.f), mtlColor.m_falloff));
		b += shadowMask * fatt * (mtlColor.m_kd * mtlColor.m_odb * std::max(nl, 0.f) + mtlColor.m_ks * mtlColor.m_osb * std::pow(std::max(nh, 0.f), mtlColor.m_falloff));
	}
	rtColor ans(r, g, b);
	return ans;
}

void rayTracer::OutputFinalImage()
{
	std::string fileName = m_fileReader->getFileName();
	for (int c = fileName.length() - 1; c >= 0; c--)
	{
		if (fileName[c] != '.')
		{
			fileName.pop_back();
		}
		else
		{
			fileName.pop_back();
			break;
		}
	}
	std::string outfileName = fileName + ".ppm";
	std::ofstream outfile(outfileName);
	outfile << "P3\n";
	auto fileInfo = m_fileReader->getFileInfo();
	outfile << static_cast<int>(fileInfo->imageSize.m_x) << ' ' << static_cast<int>(fileInfo->imageSize.m_y) << '\n';
	outfile << "255\n";

	// output the whole img
	for (int j = 0; j < static_cast<int>(fileInfo->imageSize.m_y); j++)
	{
		for (int i = 0; i < static_cast<int>(fileInfo->imageSize.m_x); i++)
		{
			if ((i + j * static_cast<int>(fileInfo->imageSize.m_x)) != 0 && (i + j * static_cast<int>(fileInfo->imageSize.m_x)) % 5 == 0)
			{
				outfile << "\n"; // 5 pixels one line
			}
			outfile << m_pixels[i][j].rtoi() << " " << m_pixels[i][j].gtoi() << " " << m_pixels[i][j].btoi() << " ";
		}
	}
	outfile.close();
}