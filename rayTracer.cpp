#include "rayTracer.h"
#include "PpmFileReader.h"
#include <iostream>

#include <cmath>
#include <corecrt_math_defines.h>

static constexpr int MAX_RECURSIVE_DEPTH = 7;
static constexpr float EPSILON = 0.00000005;

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
			std::unique_ptr<PpmFileReader> ppmFileReader = std::make_unique<PpmFileReader>(texName);
			std::vector<rtColor> texture;
			rtVector2 size;
			ppmFileReader->getTextureArray(texture, size);
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
			rtMaterial tempMtl((float)texelColor.m_r / 255.f, (double)texelColor.m_g / 255.f, (double)texelColor.m_b / 255.f,
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
	rtColor ans;
	rtPoint center;

	// set intial color based on material property
	double r = mtlColor.m_ka * mtlColor.m_odr;
	double g = mtlColor.m_ka * mtlColor.m_odg;
	double b = mtlColor.m_ka * mtlColor.m_odb;
	int n_spotligs = 0;

	// translate valid spotlights to normal lights for future use
	for (int i = 0; i < spotlights.size(); i++)
	{

		Spotlight this_spot = spotlights[i];
		Space_vector vobj;
		Space_vector vlight;
		Point this_ori;
		Color this_color;
		this_color.set_color(this_spot.r, this_spot.g, this_spot.b);
		this_ori.set_point(this_spot.x, this_spot.y, this_spot.z);
		vobj = intersection.subtract(this_ori).two_norm();
		vlight.set_vector(this_spot.dirx, this_spot.diry, this_spot.dirz);
		vlight = vlight.two_norm();

		// check if this spotlight is valid for the single intersection 
		// then add them to the global lights vector and count the number 
		if (vlight * vobj >= cos(this_spot.theta * M_PI / (double)180))
		{
			Light temp_light;
			temp_light.set_Light(this_spot.x, this_spot.y, this_spot.z, 1, this_color, this_spot.c1, this_spot.c2, this_spot.c3);
			n_spotligs++;
			lights.push_back(temp_light);
		}
	}

	// shoot lights to the intersection
	for (int i = 0; i < lights.size(); i++)
	{
		double shadow_flag = 1.0;
		Space_vector L;
		double max_t1;
		double fatt = 1;

		// calculate the L vector in phong equation
		if (lights[i].type == 0)
		{
			L.set_vector(-lights[i].x, -lights[i].y, -lights[i].z);
			L = L.two_norm();
		}
		else
		{
			Point light_source;
			light_source.set_point(lights[i].x, lights[i].y, lights[i].z);
			L = light_source.subtract(intersection);
			double length_ori_L = L.length();
			L = L.two_norm();
			double length_unit_L = L.length();
			max_t1 = length_ori_L / length_unit_L;

			// for point light, use fatt to indicate "Light Source Attenuation"
			fatt = (double)1 / (lights[i].c1 + lights[i].c2 * length_ori_L + lights[i].c3 * length_ori_L * length_ori_L);
		}

		// calculate the V and H vectors in phong equation
		Space_vector V = new_ori.subtract(intersection).two_norm();
		Space_vector H = L.add(V).two_norm();
		double nl = normal * L;
		double nh = normal * H;

		// shoot shadow rays to check shadow
		double t1 = numeric_limits<double>::infinity();
		Ray shadow_ray;
		shadow_ray.set_ray(intersection, L);
		double x0 = shadow_ray.x;
		double y0 = shadow_ray.y;
		double z0 = shadow_ray.z;
		double xd = shadow_ray.dx;
		double yd = shadow_ray.dy;
		double zd = shadow_ray.dz;
		for (int k = 0; k < spheres.size(); k++)
		{
			if (k == n_sphere && is_sphere)
			{
				continue;
			}

			double xc = spheres[k].x;
			double yc = spheres[k].y;
			double zc = spheres[k].z;
			double r = spheres[k].r;
			double A = 1.0;
			double B = 2.0 * (xd * (x0 - xc) + yd * (y0 - yc) + zd * (z0 - zc));
			double C = (x0 - xc) * (x0 - xc) + (y0 - yc) * (y0 - yc) + (z0 - zc) * (z0 - zc) - r * r;
			double delta = B * B - 4.0 * A * C;
			// check discriminant
			if (delta == 0)
			{
				double temp_t1 = (-B) / (2.0 * A);
				if (temp_t1 > 0)
				{
					if (temp_t1 < t1)
					{
						t1 = temp_t1;
					}

				}
			}
			else if (delta > 0)
			{
				double temp_t1 = (sqrt(delta) - B) / (2.0 * A);
				double temp_t2 = (-sqrt(delta) - B) / (2.0 * A);

				if (lights[i].type == 0 && ((temp_t1 > 0 && temp_t1 != numeric_limits<double>::infinity()) || (temp_t2 > 0 && temp_t2 != numeric_limits<double>::infinity())))
				{
					shadow_flag = shadow_flag * (1.0 - n_mtlcolors[spheres[k].get_m()].get_alpha());;
				}
				else if (lights[i].type == 1 && ((temp_t1 > 0 && temp_t1 < max_t1) || (temp_t2 > 0 && temp_t2 < max_t1)))
				{
					shadow_flag = shadow_flag * (1.0 - n_mtlcolors[spheres[k].get_m()].get_alpha());;
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
		for (int index_tri = 0; index_tri < faces.size(); index_tri++)
		{
			if (index_tri == n_sphere && (!is_sphere))
			{
				continue;
			}
			// same logic when shooting viewing ray
			Point first_vertex = vertices[faces[index_tri][0][0] - 1];
			Point second_vertex = vertices[faces[index_tri][1][0] - 1];
			Point third_vertex = vertices[faces[index_tri][2][0] - 1];
			Space_vector e1 = second_vertex.subtract(first_vertex);
			Space_vector e2 = third_vertex.subtract(first_vertex);
			Space_vector n_for_tri = e1.cross_prod(e2);
			double A_for_tri = n_for_tri.x;
			double B_for_tri = n_for_tri.y;
			double C_for_tri = n_for_tri.z;
			double D_for_tri = -(first_vertex.x * A_for_tri + first_vertex.y * B_for_tri + first_vertex.z * C_for_tri);
			double deterimint = A_for_tri * xd + B_for_tri * yd + C_for_tri * zd;
			if (deterimint <= 0.0000005 && deterimint >= -0.0000005)
			{
				continue;
			}
			double t_for_tri = -(A_for_tri * x0 + B_for_tri * y0 + C_for_tri * z0 + D_for_tri) / deterimint;
			if (t_for_tri < 0)
			{
				continue;
			}
			Point p_for_tri = intersection.p_add(shadow_ray.raydir().scale(t_for_tri));
			Space_vector e3 = p_for_tri.subtract(second_vertex);
			Space_vector e4 = p_for_tri.subtract(third_vertex);
			double totalArea = e1.Area(e2);
			double aArea = e3.Area(e4);
			double bArea = e4.Area(e2);
			double cArea = e1.Area(e3);
			double alpha = aArea / totalArea;
			double beta = bArea / totalArea;
			double gamma = cArea / totalArea;
			if (alpha < 1 && alpha>0 && beta < 1 && beta>0 && gamma > 0 && gamma < 1 && alpha + beta + gamma - 1 < 0.00005)
			{

				if (lights[i].type == 0 && t_for_tri > 0 && t_for_tri != numeric_limits<double>::infinity())
				{
					shadow_flag = shadow_flag * (1.0 - n_mtlcolors[face_mtl[index_tri]].get_alpha());
				}
				else if (lights[i].type == 1 && t_for_tri > 0 && t_for_tri < max_t1)
				{
					shadow_flag = shadow_flag * (1.0 - n_mtlcolors[face_mtl[index_tri]].get_alpha());
				}


				if (t_for_tri < t1)
				{
					t1 = t_for_tri;
				}

			}


		}


		// using phong equation to calculate rgb values
		r += shadow_flag * fatt * (mtlc.kd * mtlc.odr * max(nl, (double)0) + mtlc.ks * mtlc.osr * pow(max(nh, (double)0), mtlc.falloff));
		g += shadow_flag * fatt * (mtlc.kd * mtlc.odg * max(nl, (double)0) + mtlc.ks * mtlc.osg * pow(max(nh, (double)0), mtlc.falloff));
		b += shadow_flag * fatt * (mtlc.kd * mtlc.odb * max(nl, (double)0) + mtlc.ks * mtlc.osb * pow(max(nh, (double)0), mtlc.falloff));
	}
	ans.set_color(r, g, b);

	//release all the spotlights in lights vector
	for (int i = 0; i < n_spotligs; i++)
	{
		lights.pop_back();
	}
	return ans;
}