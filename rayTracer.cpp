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

		if (delta <= 0.00000005 && delta >= -0.00000005)// (delta close to zero)
		{
			float tempT = (-B) / (2.f * A);
			if (tempT < t1 && tempT > 0.00000005)
			{
				t1 = tempT;
				objIndex = sphereIndex;
			}
			else 
			{
				total++;
			}
		}
		else if (delta > 0.00000005)
		{
			float tempT1 = (std::sqrt(delta) - B) / (2.f * A);
			float tempT2 = (-std::sqrt(delta) - B) / (2.f * A);
			bool isHit = false;
			if ((tempT1 < t1) && tempT1 > 0.00000005)
			{
				t1 = tempT1;
				objIndex = sphereIndex;
				isHit = true;
			}

			if ((tempT2 < t1) && tempT2 > 0.00000005)
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
		rtVector3 n_for_tri = rtVector3::crossProduct(e1, e2);
		float A_for_tri = n_for_tri.m_x;
		float B_for_tri = n_for_tri.m_y;
		float C_for_tri = n_for_tri.m_z;
		float D_for_tri = -(firstVertex.m_x * A_for_tri + firstVertex.m_y * B_for_tri + firstVertex.m_z * C_for_tri);
		float deterimint = A_for_tri * incidence.m_direction.m_x + B_for_tri * incidence.m_direction.m_y + C_for_tri * incidence.m_direction.m_z;
		// check if viewdir is parallel to the surface
		if (deterimint < 0.00000005 && deterimint > -0.00000005)
		{
			total++;
			continue;
		}
		float t_for_tri = -(A_for_tri * incidence.m_origin.m_x + B_for_tri * incidence.m_origin.m_y + C_for_tri * incidence.m_origin.m_z + D_for_tri) / deterimint;
		if (t_for_tri < 0.f || t_for_tri > t1)
		{
			total++;
			continue;
		}
		rtPoint p_for_tri = incidence.m_origin.add(incidence.raydir().scale(t_for_tri));
		Space_vector e3 = p_for_tri.subtract(second_vertex);
		Space_vector e4 = p_for_tri.subtract(third_vertex);
		double totalArea = e1.Area(e2);
		double aArea = e3.Area(e4);
		double bArea = e4.Area(e2);
		double cArea = e1.Area(e3);
		double alpha = aArea / totalArea;
		double beta = bArea / totalArea;
		double gamma = cArea / totalArea;
		// determine Barycentric coordinates
		if (alpha < 1 && alpha>0 && beta < 1 && beta>0 && gamma > 0 && gamma < 1 && alpha + beta + gamma - 1 < 0.00005)
		{
			t1 = t_for_tri;
			is_sphere = false;
			which_object = index_tri;
			alphas.push_back(alpha);
			betas.push_back(beta);
			gammas.push_back(gamma);
			if (faces[index_tri][0][2] == 0)
			{
				// if no vn, which means flat shading
				n_for_tris.push_back(n_for_tri);
			}
			else
			{
				// smooth shading
				Space_vector first_nromal = vertex_normals[faces[index_tri][0][2] - 1];
				Space_vector second_nromal = vertex_normals[faces[index_tri][1][2] - 1];
				Space_vector third_nromal = vertex_normals[faces[index_tri][2][2] - 1];
				Space_vector smooth_normal = (first_nromal.scale(alpha).add(second_nromal.scale(beta)).add(third_nromal.scale(gamma))).two_norm();
				n_for_tris.push_back(smooth_normal);
			}

		}
		else
		{
			total++;
			continue;
		}

	}
	// if we detect an intersection,find this point and change img color
	if (total != spheres.size() + faces.size())
	{
		//cout << spheres.size()<<"/"<<faces.size() << endl;
		Space_vector I = incidence.raydir().two_norm().scale(-1);
		Space_vector d_ray;
		d_ray = incidence.raydir().scale(t1);
		Point closest = incidence.ori().p_add(d_ray);
		Mtlcolor temp;
		Space_vector normal;
		Point sphere_center;
		if (is_sphere)
		{
			// compute normal vector for sphere which will be used in phong equation
			temp = n_mtlcolors[spheres[which_object].get_m()];
			sphere_center.set_point(spheres[which_object].x, spheres[which_object].y, spheres[which_object].z);
			normal = closest.subtract(sphere_center).two_norm();
		}
		else
		{
			// compute normal vector for triangle which will be used in phong equation
			temp = n_mtlcolors[face_mtl[which_object]];
			normal = n_for_tris.back().two_norm();
		}

		if (I * normal < 0)
		{
			normal = normal.scale(-1);

		}

		if (is_sphere && (is_sphere == is_sphere_) && (which_object == which_object_))
		{
			exit = true;
		}

		string name = temp.get_texturefile();
		if (name != "") // if texture detected
		{

			int index_of_texture = texturefilename_to_index[name];
			vector<int> from_texture;
			if (!is_sphere)
			{
				//mapping texture to a triangle
				vector<double> first_2d = vertex_textures[faces[which_object][0][1] - 1];
				vector<double> second_2d = vertex_textures[faces[which_object][1][1] - 1];
				vector<double> third_2d = vertex_textures[faces[which_object][2][1] - 1];
				// using Barycentric coordinates
				double u_for_texture, v_for_texture;
				u_for_texture = (alphas.back() * first_2d[0] + betas.back() * second_2d[0] + gammas.back() * third_2d[0]);
				v_for_texture = (alphas.back() * first_2d[1] + betas.back() * second_2d[1] + gammas.back() * third_2d[1]);
				from_texture = arrays_of_textures[index_of_texture][(int)((v_for_texture * (size_of_textures[index_of_texture][1] - 1)) + 0.5)][(int)((u_for_texture * (size_of_textures[index_of_texture][0] - 1) + 0.5))];
				Mtlcolor this_temp;

				this_temp.set_mtlcolor((double)from_texture[0] / 255.0, (double)from_texture[1] / 255.0, (double)from_texture[2] / 255.0, temp.osr, temp.osg, temp.osb, temp.ka, temp.kd, temp.ks, temp.falloff, temp.get_alpha(), temp.get_eta());
				hit = Phong(this_temp, closest, which_object, normal, is_sphere, incidence.ori());
			}
			else
			{
				//mapping texture to a sphere
				double phi = acos((closest.z - spheres[which_object].z) / spheres[which_object].r);
				double zeta = atan2((closest.y - spheres[which_object].y), (closest.x - spheres[which_object].x));
				double u_for_texture, v_for_texture;
				v_for_texture = phi / M_PI;
				u_for_texture = (zeta + M_PI) / (2 * M_PI);
				from_texture = arrays_of_textures[index_of_texture][(int)((v_for_texture * (size_of_textures[index_of_texture][1] - 1)) + 0.5)][(int)((u_for_texture * (size_of_textures[index_of_texture][0] - 1) + 0.5))];
				Mtlcolor this_temp;


				this_temp.set_mtlcolor((double)from_texture[0] / 255.0, (double)from_texture[1] / 255.0, (double)from_texture[2] / 255.0, temp.osr, temp.osg, temp.osb, temp.ka, temp.kd, temp.ks, temp.falloff, temp.get_alpha(), temp.get_eta());
				hit = Phong(this_temp, closest, which_object, normal, is_sphere, incidence.ori());

			}
		}
		else // no texture detected, apply normal phong equation
		{
			hit = Phong(temp, closest, which_object, normal, is_sphere, incidence.ori());
		}


		Space_vector litter_for = incidence.raydir().scale(t1 + 0.00005);
		Point forward = incidence.ori().p_add(litter_for);

		Space_vector litter_bac = incidence.raydir().scale(t1 - 0.00005);
		Point backward = incidence.ori().p_add(litter_bac);

		Ray reflection;
		Space_vector reflection_dir;

		double cosphii = abs(I * normal);
		double sinphii = pow(1.0 - cosphii * cosphii, 0.5);
		reflection_dir = normal.scale(cosphii * 2.0).add(incidence.raydir().two_norm());
		reflection.set_ray(backward, reflection_dir);

		double this_eta = temp.get_eta();
		double this_alpha = temp.get_alpha();

		/*
		if (is_sphere)
		{
			this_eta = n_mtlcolors[spheres[which_object].get_m()].get_eta();
			this_alpha = n_mtlcolors[spheres[which_object].get_m()].get_alpha();
		}
		else
		{
			this_eta = n_mtlcolors[face_mtl[which_object]].get_eta();
			this_alpha = n_mtlcolors[face_mtl[which_object]].get_alpha();
		}
		*/
		if (exit)
		{
			this_eta = last_eta;
		}
		double F0 = ((this_eta - etai) / (this_eta + etai)) * ((this_eta - etai) / (this_eta + etai));
		double Fresnel_reflectance = F0 + (1.0 - F0) * pow((1.0 - cosphii), 5.0);

		Ray transmission;
		Space_vector transmission_dir;
		double first_coff = pow(1.0 - (etai / this_eta) * (etai / this_eta) * (1.0 - cosphii * cosphii), 0.5);
		transmission_dir = normal.scale(-1).scale(first_coff).add(normal.scale(cosphii * (etai / this_eta))).add(I.scale(-1).scale(etai / this_eta));
		transmission_dir = transmission_dir.two_norm();
		if (!is_sphere)
		{
			transmission_dir = incidence.raydir().two_norm();
		}

		//cout << (incidence.raydir().two_norm()) * transmission_dir << endl;

		transmission.set_ray(forward, transmission_dir);

		if (is_sphere)
		{
			if (sinphii > (this_eta / etai))
			{
				hit = hit + (trace_ray(reflection, rec_depth + 1, etai, is_sphere, which_object, etai) * Fresnel_reflectance);
			}
			else
			{
				Color trans;
				trans = trace_ray(transmission, rec_depth + 1, this_eta, is_sphere, which_object, etai) * ((1.0 - Fresnel_reflectance) * (1.0 - this_alpha));
				hit = hit + (trace_ray(reflection, rec_depth + 1, etai, is_sphere, which_object, etai) * Fresnel_reflectance) + trans;
			}
		}
		else
		{
			if (sinphii > (this_eta / etai))
			{
				hit = hit + (trace_ray(reflection, rec_depth + 1, etai, is_sphere, which_object, etai) * Fresnel_reflectance);
			}
			else
			{
				Color trans;
				trans = trace_ray(transmission, rec_depth + 1, etai, is_sphere, which_object, etai) * ((1.0 - Fresnel_reflectance) * (1.0 - this_alpha));
				hit = hit + (trace_ray(reflection, rec_depth + 1, etai, is_sphere, which_object, etai) * Fresnel_reflectance) + trans;
			}
		}
	}
	else
	{
		return bkgcolor;
	}

	return hit;
}