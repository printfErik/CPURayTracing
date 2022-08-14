#include "rayTracer.h"
#include "PpmFileReader.h"
#include <iostream>
#include <filesystem>
#include <cmath>
#include <corecrt_math_defines.h>

static constexpr int MAX_RECURSIVE_DEPTH = 7;
static constexpr double EPSILON = 0.00000005;

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
			rtVector2<int> size;
			ppmFileReaderInstance->getTextureArray(texture, size);
			m_textureData[texName] = texture;
			m_textureSize[texName] = size;
		}
	}
	return true;
}

bool rayTracer::ComputeUV()
{
	auto fileInfo = m_fileReader->getFileInfo();

	// check viewDir and upDir are not parallel
	m_u = rtVector3::crossProduct(fileInfo->viewDir, fileInfo->upDir);

	if (m_u.m_x == 0.0 && m_u.m_y == 0.0 && m_u.m_z == 0.0)
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

	double aspectRatio = fileInfo->imageSize.m_x / fileInfo->imageSize.m_y;
	double distance = 5.0; // random number
	double height = 2.0 * distance * std::tan(fileInfo->vFov * M_PI / 360.0);
	double width = height * aspectRatio;
	
	rtVector3 n = fileInfo->viewDir.getTwoNorm();
	rtPoint center = rtPoint::add(fileInfo->eye, n.scale(distance));
	m_ul = rtPoint::add(rtPoint::add(center, m_u.scale(-width / 2.0)), m_v.scale(height / 2.0));
	m_ur = rtPoint::add(fileInfo->eye, n.scale(distance).add(m_u.scale(width / 2.0).add(m_v.scale(height / 2.0))));
	m_ll = rtPoint::add(fileInfo->eye, n.scale(distance).add(m_u.scale(-width / 2.0).add(m_v.scale(-height / 2.0))));
	m_lr = rtPoint::add(fileInfo->eye, n.scale(distance).add(m_u.scale(width / 2.0).add(m_v.scale(-height / 2.0))));

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
	rtVector3 d_h = m_ur.subtract(m_ul).scale(1.0 / static_cast<double>(fileInfo->imageSize.m_x));
	rtVector3 d_v = m_ll.subtract(m_ul).scale(1.0 / static_cast<double>(fileInfo->imageSize.m_y));

	rtVector3 d_ch = m_ur.subtract(m_ul).scale(1.0 / (2.0 * static_cast<double>(fileInfo->imageSize.m_x)));
	rtVector3 d_cv = m_ll.subtract(m_ul).scale(1.0 / (2.0 * static_cast<double>(fileInfo->imageSize.m_y)));
	
	for (int i = 0; i < fileInfo->imageSize.m_x; i++)
	{
		for (int j = 0; j < fileInfo->imageSize.m_y; j++)
		{
			rtPoint p = rtPoint::add(rtPoint::add(rtPoint::add(rtPoint::add(m_ul, d_h.scale((double)i)), d_v.scale((double)j)), d_ch), d_cv);
			rtVector2<int> index(i, j);
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
			rtVector2<int> index(i, j);
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
			rtVector2<int> index(i, j);
			rtRay ray = m_imgIndex2RayMap[index];
			rtColor pixelColor = RecursiveTraceRay(ray, 0, 1.0, true, -1, 1.0);
			pixelColor.clamp();
			m_pixels[i][j] = pixelColor;
		}
	}
}

rtColor rayTracer::RecursiveTraceRay(const rtRay& incidence, int recusiveDepth, double etai, bool isSphere, int objIndex, double lastEta)
{
	auto fileInfo = m_fileReader->getFileInfo();
	if (recusiveDepth == MAX_RECURSIVE_DEPTH)
	{
		double r, g, b;
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
	double t1 = std::numeric_limits<double>::infinity();

	std::vector<rtVector3> triNormals;

	std::vector<double> alphas, betas, gammas;

	bool isSphere_ = true;
	int objIndex_ = -1;
	
	int total = 0;

	for (int sphereIndex = 0; sphereIndex < fileInfo->spheres.size(); sphereIndex++)
	{
		double xc = fileInfo->spheres[sphereIndex].m_center.m_x;
		double yc = fileInfo->spheres[sphereIndex].m_center.m_y;
		double zc = fileInfo->spheres[sphereIndex].m_center.m_z;
		double r = fileInfo->spheres[sphereIndex].m_radius;

		double distanceX = incidence.m_origin.m_x - xc;
		double distanceY = incidence.m_origin.m_y - yc;
		double distanceZ = incidence.m_origin.m_z - zc;

		double A = 1.0;
		double B = 2.0 * (incidence.m_direction.m_x * distanceX +
						 incidence.m_direction.m_y * distanceY +
						 incidence.m_direction.m_z * distanceZ);
		double C = distanceX * distanceX + distanceY * distanceY + distanceZ * distanceZ - r * r;

		double delta = B * B - 4.0 * A * C;

		if (delta == 0.0)// (delta close to zero)
		{
			double tempT = (-B) / (2.0 * A);
			if (tempT < t1 && tempT > 0.0000005)
			{
				t1 = tempT;
				objIndex_ = sphereIndex;
			}
			else 
			{
				total++;
			}
		}
		else if (delta > 0)
		{
			double tempT1 = (std::sqrt(delta) - B) / (2.0 * A);
			double tempT2 = (-std::sqrt(delta) - B) / (2.0 * A);
			bool isHit = false;
			if ((tempT1 < t1) && tempT1 > 0.0000005)
			{
				t1 = tempT1;
				objIndex_ = sphereIndex;
				isHit = true;
			}

			if ((tempT2 < t1) && tempT2 > 0.0000005)
			{
				t1 = tempT2;
				objIndex_ = sphereIndex;
				isHit = true;
			}

			if (!isHit)
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
		double A = normal.m_x;
		double B = normal.m_y;
		double C = normal.m_z;
		double D = -(firstVertex.m_x * A + firstVertex.m_y * B + firstVertex.m_z * C);
		double deterimint = A * incidence.m_direction.m_x + B * incidence.m_direction.m_y + C * incidence.m_direction.m_z;

		// check if viewdir is parallel to the surface
		if (deterimint <= EPSILON && deterimint >= -EPSILON)
		{
			total++;
			continue;
		}
		double tTri = -(A * incidence.m_origin.m_x + B * incidence.m_origin.m_y + C * incidence.m_origin.m_z + D) / deterimint;
		if (tTri < 0.0 || tTri > t1)
		{
			total++;
			continue;
		}
		rtPoint hitPoint = rtPoint::add(incidence.m_origin, incidence.m_direction.scale(tTri));
		rtVector3 e3 = hitPoint.subtract(secondVertex);
		rtVector3 e4 = hitPoint.subtract(thirdVertex);
		double totalArea = rtVector3::area(e1, e2);
		double aArea = rtVector3::area(e3, e4);
		double bArea = rtVector3::area(e4, e2);
		double cArea = rtVector3::area(e1, e3);
		double alpha = aArea / totalArea;
		double beta = bArea / totalArea;
		double gamma = cArea / totalArea;

		// determine Barycentric coordinates
		if (alpha < 1 && alpha > 0 && beta < 1 && beta > 0 && gamma > 0 && gamma < 1 && alpha + beta + gamma - 1 < EPSILON)
		{
			t1 = tTri;
			isSphere_ = false;
			objIndex_ = triangleIndex;
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
	if (total != (fileInfo->spheres.size() + fileInfo->faces.size()))
	{
		rtVector3 I = incidence.m_direction.getTwoNorm().scale(-1);
		rtVector3 rayDir = incidence.m_direction.scale(t1);
		rtPoint closest = rtPoint::add(incidence.m_origin, rayDir);
		rtMaterial temp;
		rtVector3 normal;
		if (isSphere_)
		{
			// compute normal vector for sphere which will be used in phong equation
			temp = fileInfo->materials[fileInfo->spheres[objIndex_].m_materialIndex];
			normal = closest.subtract(fileInfo->spheres[objIndex_].m_center).getTwoNorm();
		}
		else
		{
			// compute normal vector for triangle which will be used in phong equation
			temp = fileInfo->materials[fileInfo->faceMaterialIndexs[objIndex_]];
			normal = triNormals.back().getTwoNorm();
		}

		if (rtVector3::dotProduct(I, normal) < 0)
		{
			normal = normal.scale(-1);
		}

		if (isSphere_ && (isSphere_ == isSphere) && (objIndex == objIndex_))
		{
			exit = true;
		}

		std::string name = temp.getTextureFile();
		if (!name.empty()) // if texture detected
		{
			rtColor texelColor;
			double textureU, textureV;
			if (!isSphere_)
			{
				//mapping texture to a triangle
				rtVector2<double> first_2d = fileInfo->vertexTextureCoordinates[fileInfo->faces[objIndex_][0][1] - 1];
				rtVector2<double> second_2d = fileInfo->vertexTextureCoordinates[fileInfo->faces[objIndex_][1][1] - 1];
				rtVector2<double> third_2d = fileInfo->vertexTextureCoordinates[fileInfo->faces[objIndex_][2][1] - 1];
				// using Barycentric coordinates
				textureU = (alphas.back() * first_2d.m_x + betas.back() * second_2d.m_x + gammas.back() * third_2d.m_x);
				textureV = (alphas.back() * first_2d.m_y + betas.back() * second_2d.m_y + gammas.back() * third_2d.m_y);
			}
			else
			{
				//mapping texture to a sphere
				double phi = std::acos((closest.m_z - fileInfo->spheres[objIndex_].m_center.m_z) / fileInfo->spheres[objIndex_].m_radius);
				double zeta = std::atan2((closest.m_y - fileInfo->spheres[objIndex_].m_center.m_y), (closest.m_x - fileInfo->spheres[objIndex_].m_center.m_x));
				textureV = phi / static_cast<double>(M_PI);
				textureU = (zeta + static_cast<double>(M_PI)) / (2.0 * static_cast<double>(M_PI));
			}
			texelColor = m_textureData[name][static_cast<int>(textureV * (m_textureSize[name].m_y - 1.0) + 0.5) * static_cast<int>(m_textureSize[name].m_x)
										   + static_cast<int>(textureU * (m_textureSize[name].m_x - 1.0) + 0.5)];
			rtMaterial tempMtl(texelColor.m_r / 255.0, texelColor.m_g / 255.0, texelColor.m_b / 255.0,
								temp.m_osr, temp.m_osg, temp.m_osb,
								temp.m_ka, temp.m_kd, temp.m_ks, temp.m_falloff, temp.m_alpha, temp.m_eta);
			hit = BlinnPhongShading(tempMtl, closest, objIndex_, normal, isSphere_, incidence.m_origin);
		}
		else // no texture detected, apply normal phong equation
		{
			hit = BlinnPhongShading(temp, closest, objIndex_, normal, isSphere_, incidence.m_origin);
		}

		
		rtVector3 forwardEpsilon = incidence.m_direction.scale(t1 + 0.005);
		rtPoint forward = rtPoint::add(incidence.m_origin, forwardEpsilon);

		rtVector3 backwardEpsilon = incidence.m_direction.scale(t1 - 0.005);
		rtPoint backward = rtPoint::add(incidence.m_origin, backwardEpsilon);

		rtRay reflection;
		rtVector3 reflectionDir;

		double cosphii = std::abs(rtVector3::dotProduct(I, normal));
		double sinphii = std::pow(1.0 - cosphii * cosphii, 0.5);
		reflectionDir = normal.scale(cosphii * 2.0).add(incidence.m_direction.getTwoNorm());
		reflection.m_origin = backward;
		reflection.m_direction = reflectionDir;

		double curEta = temp.m_eta;
		double curAlpha = temp.m_alpha;

		if (exit)
		{
			curEta = lastEta;
		}
		double F0 = ((curEta - etai) / (curEta + etai)) * ((curEta - etai) / (curEta + etai));
		double FresnelReflectance = F0 + (1.0 - F0) * std::pow((1.0 - cosphii), 5.0);

		rtRay transmission;
		rtVector3 transmissionDir;
		if (!isSphere_)
		{
			transmissionDir = incidence.m_direction.getTwoNorm();
		}
		else
		{
			double firstCoff = std::pow(1.0 - (etai / curEta) * (etai / curEta) * (1.0 - cosphii * cosphii), 0.5);
			transmissionDir = normal.scale(-1.0).scale(firstCoff).add(normal.scale(cosphii * (etai / curEta))).add(I.scale(-1.0).scale(etai / curEta));
			transmissionDir.twoNorm();
		}

		transmission.m_origin = forward;
		transmission.m_direction = transmissionDir;

		if (sinphii > (curEta / etai))
		{
			hit = hit + (RecursiveTraceRay(reflection, recusiveDepth + 1, etai, isSphere_, objIndex_, etai) * FresnelReflectance);
		}
		else
		{
			rtColor trans;
			trans = RecursiveTraceRay(transmission, recusiveDepth + 1, isSphere_ ? curEta : etai, isSphere_, objIndex_, etai) * ((1.0 - FresnelReflectance) * (1.0 - curAlpha));
			hit = hit + (RecursiveTraceRay(reflection, recusiveDepth + 1, etai, isSphere_, objIndex_, etai) * FresnelReflectance) + trans;
		}
		
	}
	else
	{
		return fileInfo->bkgColor;
	}

	return hit;
}

rtColor rayTracer::BlinnPhongShading(const rtMaterial& mtlColor, const rtPoint& intersection, int objIndex, const rtVector3& normal, bool isSphere, const rtPoint& newOrigin)
{
	auto fileInfo = m_fileReader->getFileInfo();
	rtPoint center;

	// set intial color based on material property
	double r = mtlColor.m_ka * mtlColor.m_odr;
	double g = mtlColor.m_ka * mtlColor.m_odg;
	double b = mtlColor.m_ka * mtlColor.m_odb;

	// shoot lights to the intersection
	for (int i = 0; i < fileInfo->lights.size(); i++)
	{
		auto curLight = fileInfo->lights[i];
		double shadowMask = 1.0;
		rtVector3 lightDir;
		double maxT1 = 0.0;
		double fatt = 1.0;

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
			fatt = 1.0 / (curLight.m_c1 + curLight.m_c2 * maxT1 + curLight.m_c3 * maxT1 * maxT1);
		}
		break;
		case eLightType::kDirectionalLight:
		{
			lightDir = rtVector3(-curLight.m_vec3.m_x, -curLight.m_vec3.m_y, -curLight.m_vec3.m_z);
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
			if (rtVector3::dotProduct(vlight, vobj) >= std::cos(curLight.m_theta * M_PI / 180.0))
			{
				rtPoint lightSource(curLight.m_center.m_x, curLight.m_center.m_y, curLight.m_center.m_z);
				lightDir = lightSource.subtract(intersection);
				maxT1 = lightDir.length();

				// for point light, use fatt to indicate "Light Source Attenuation"
				fatt = 1.0 / (curLight.m_c1 + curLight.m_c2 * maxT1 + curLight.m_c3 * maxT1 * maxT1);
			}
			else
			{
				continue;
			}
		}
		break;
		default:
			break;
		}

		// calculate the V and H vectors in phong equation
		rtVector3 V = newOrigin.subtract(intersection).getTwoNorm();
		rtVector3 H = lightDir.add(V).getTwoNorm();
		double nl = rtVector3::dotProduct(normal, lightDir);
		double nh = rtVector3::dotProduct(normal, H);

		// shoot shadow rays to check shadow
		double t1 = std::numeric_limits<double>::infinity();
		rtRay shadowRay;
		shadowRay.m_origin = intersection;
		shadowRay.m_direction = lightDir;
		double x0 = shadowRay.m_origin.m_x;
		double y0 = shadowRay.m_origin.m_y;
		double z0 = shadowRay.m_origin.m_z;
		double xd = shadowRay.m_direction.m_x;
		double yd = shadowRay.m_direction.m_y;
		double zd = shadowRay.m_direction.m_z;

		for (int k = 0; k < fileInfo->spheres.size(); k++)
		{
			if (k == objIndex && isSphere)
			{
				continue;
			}

			double xc = fileInfo->spheres[k].m_center.m_x;
			double yc = fileInfo->spheres[k].m_center.m_y;
			double zc = fileInfo->spheres[k].m_center.m_z;
			double r = fileInfo->spheres[k].m_radius;

			double A = 1.0;
			double B = 2.0 * (xd * (x0 - xc) + yd * (y0 - yc) + zd * (z0 - zc));
			double C = (x0 - xc) * (x0 - xc) + (y0 - yc) * (y0 - yc) + (z0 - zc) * (z0 - zc) - r * r;
			double delta = B * B - 4.0 * A * C;

			// check discriminant
			if (delta == 0.0)
			{
				double temp_t1 = (-B) / (2.0 * A);
				if (temp_t1 > 0 && temp_t1 < t1)
				{
					t1 = temp_t1;
				}
			}
			else if (delta > 0.0)
			{
				double temp_t1 = (std::sqrt(delta) - B) / (2.0 * A);
				double temp_t2 = (-std::sqrt(delta) - B) / (2.0 * A);

				double comparator = curLight.getType() == eLightType::kDirectionalLight ? std::numeric_limits<double>::infinity() : maxT1;
				if ((temp_t1 > 0 && temp_t1 < comparator) || (temp_t2 > 0 && temp_t2 < comparator))
				{
					shadowMask = shadowMask * (1.0 - fileInfo->materials[fileInfo->spheres[k].m_materialIndex].m_alpha);
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
			double A = normal.m_x;
			double B = normal.m_y;
			double C = normal.m_z;
			double D = -(firstVertex.m_x * A + firstVertex.m_y * B + firstVertex.m_z * C);
			double deterimint = A * xd + B * yd + C * zd;
			if (deterimint <= 0.0000005 && deterimint >= -0.0000005)
			{
				continue;
			}
			double triT = -(A * x0 + B * y0 + C * z0 + D) / deterimint;
			if (triT <= 0.0)
			{
				continue;
			}
			rtPoint hitP = rtPoint::add(intersection, shadowRay.m_direction.scale(triT));
			rtVector3 e3 = hitP.subtract(secondVertex);
			rtVector3 e4 = hitP.subtract(thirdVertex);
			double totalArea = rtVector3::area(e1, e2);
			double aArea = rtVector3::area(e3, e4);
			double bArea = rtVector3::area(e4, e2);
			double cArea = rtVector3::area(e1, e3);
			double alpha = aArea / totalArea;
			double beta = bArea / totalArea;
			double gamma = cArea / totalArea;
			if (alpha < 1.0 && alpha > 0.0 && beta < 1.0 && beta > 0.0 && gamma > 0.0 && gamma < 1.0 && alpha + beta + gamma - 1 < 0.0005)
			{
				double comparator = curLight.getType() == eLightType::kDirectionalLight ? std::numeric_limits<double>::infinity() : maxT1;

				if (triT > 0 && triT < comparator)
				{
					shadowMask = shadowMask * (1.0 - fileInfo->materials[fileInfo->faceMaterialIndexs[triIndex]].m_alpha);
				}

				if (triT < t1 && triT > 0) // todo might not correct
				{
					t1 = triT;
				}
			}
		}

		// using phong equation to calculate rgb values
		r += 1.0 * fatt * (mtlColor.m_kd * mtlColor.m_odr * std::max(nl, 0.0) + mtlColor.m_ks * mtlColor.m_osr * std::pow(std::max(nh, 0.0), mtlColor.m_falloff));
		g += 1.0 * fatt * (mtlColor.m_kd * mtlColor.m_odg * std::max(nl, 0.0) + mtlColor.m_ks * mtlColor.m_osg * std::pow(std::max(nh, 0.0), mtlColor.m_falloff));
		b += 1.0 * fatt * (mtlColor.m_kd * mtlColor.m_odb * std::max(nl, 0.0) + mtlColor.m_ks * mtlColor.m_osb * std::pow(std::max(nh, 0.0), mtlColor.m_falloff));
	}
	rtColor ans(r, g, b);
	return ans;
}

void rayTracer::OutputFinalImage(const std::string& outFolderName)
{
	auto outFilePath = std::filesystem::path(m_fileReader->getFileName());
	std::string outfileName = outFolderName + "\\" + outFilePath.stem().string() + ".ppm";
	std::ofstream outfile(outfileName);
	outfile << "P3\n";
	auto fileInfo = m_fileReader->getFileInfo();
	outfile << fileInfo->imageSize.m_x << ' ' << fileInfo->imageSize.m_y << '\n';
	outfile << "255\n";

	// output the whole img
	for (int j = 0; j < fileInfo->imageSize.m_y; j++)
	{
		for (int i = 0; i < fileInfo->imageSize.m_x; i++)
		{
			if ((i + j * fileInfo->imageSize.m_x) != 0 && (i + j * fileInfo->imageSize.m_x) % 5 == 0)
			{
				outfile << "\n"; // 5 pixels one line
			}
			outfile << m_pixels[i][j].rtoi() << " " << m_pixels[i][j].gtoi() << " " << m_pixels[i][j].btoi() << " ";
		}
	}
	outfile.close();
}