#include "ObjFileReader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

ObjFileReader::ObjFileReader()
{
	m_objFileInfo = std::make_shared<ObjFileInfo>();
}

ObjFileReader::ObjFileReader(const std::string& _fileName)
{
	m_objFileInfo = std::make_shared<ObjFileInfo>();
	m_fileName = _fileName;
}

std::shared_ptr<ObjFileInfo> ObjFileReader::getFileInfo()
{
	return m_objFileInfo;
}

std::string ObjFileReader::getFileName() 
{
	return m_fileName;
}

eParseRetType ObjFileReader::parseFile()
{
	_ASSERT(m_objFileInfo);
	std::ifstream inFile;
	inFile.open(m_fileName);

	if (inFile.fail())
	{
		std::cout << "Can't find this file: " << m_fileName << std::endl;
		return eParseRetType::kFileNotExists;
	}

	std::string line;

	bool hasEye = false;
	bool hasViewDir = false; 
	bool hasUpDir = false;
	bool hasVFov = false;
	bool hasImgSize = false;
	bool hasBkgColor = false;
	while (std::getline(inFile, line))
	{
		std::istringstream iss(line);
		std::string block;
		while (iss >> block)
		{
			if (block == "eye")
			{
				std::vector<double> vec(3);
				for (int i = 0; i < 3; i++)
				{
					if (iss >> block)
					{
						vec[i] = std::stod(block);
					}
					else
					{
						std::cout << "-----------FILE PARSE ERROR-------------"<< std::endl;
						std::cout << "Keyword: " << block << " requires 3 doubles" << std::endl;
						return eParseRetType::kEyeKeywordFormatError;
					}
				}
				m_objFileInfo->eye = rtPoint(vec[0], vec[1], vec[2]);
				hasEye = true;
			}
			else if (block == "viewdir")
			{
				std::vector<double> vec(3);
				for (int i = 0; i < 3; i++)
				{
					if (iss >> block)
					{
						vec[i] = std::stod(block);
					}
					else
					{
						std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
						std::cout << "Keyword: " << block << " requires 3 doubles" << std::endl;
						return eParseRetType::kEyeKeywordFormatError;
					}
				}
				m_objFileInfo->viewDir = rtVector3(vec[0], vec[1], vec[2]);
				hasViewDir = true;
			}
			else if (block == "updir")
			{
				std::vector<double> vec(3);
				for (int i = 0; i < 3; i++)
				{
					if (iss >> block)
					{
						vec[i] = std::stod(block);
					}
					else
					{
						std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
						std::cout << "Keyword: " << block << " requires 3 doubles" << std::endl;
						return eParseRetType::kEyeKeywordFormatError;
					}
				}
				m_objFileInfo->upDir = rtVector3(vec[0], vec[1], vec[2]);
				hasUpDir = true;
			}
			else if (block == "vfov")
			{
				if (iss >> block)
				{
					m_objFileInfo->vFov = std::stod(block);
				}
				else
				{
					std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
					std::cout << "Keyword: " << block << " requires 3 doubles" << std::endl;
					return eParseRetType::kEyeKeywordFormatError;
				}
				hasVFov = true;
			}
			else if (block == "imsize")
			{
				std::vector<int> vec(2);
				for (int i = 0; i < 2; i++)
				{
					if (iss >> block)
					{
						vec[i] = std::stoi(block);
					}
					else
					{
						std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
						std::cout << "Keyword: " << block << " requires 2 doubles" << std::endl;
						return eParseRetType::kEyeKeywordFormatError;
					}
				}
				m_objFileInfo->imageSize = rtVector2<int>(vec[0], vec[1]);
				hasImgSize = true;
			}
			else if (block == "bkgcolor")
			{
				std::vector<double> vec(3);
				for (int i = 0; i < 3; i++)
				{
					if (iss >> block)
					{
						vec[i] = std::stod(block);
					}
					else
					{
						std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
						std::cout << "Keyword: " << block << " requires 3 doubles" << std::endl;
						return eParseRetType::kEyeKeywordFormatError;
					}
				}
				m_objFileInfo->bkgColor = rtColor(vec[0], vec[1], vec[2]);
				hasBkgColor = true;
			}
			else if (block == "mtlcolor")
			{
				std::vector<double> vec(12);
				for (int i = 0; i < 12; i++)
				{
					if (iss >> block)
					{
						vec[i] = std::stod(block);
					}
					else
					{
						std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
						std::cout << "Keyword: " << block << " requires 12 doubles" << std::endl;
						return eParseRetType::kEyeKeywordFormatError;
					}
				}
				m_objFileInfo->materials.push_back(rtMaterial(vec[0], vec[1], vec[2], vec[3], vec[4], vec[5], vec[6], vec[7], vec[8], vec[9], vec[10], vec[11]));
			}
			else if (block == "sphere")
			{
				std::vector<double> vec(4);
				for (int i = 0; i < 4; i++)
				{
					if (iss >> block)
					{
						vec[i] = std::stod(block);
					}
					else
					{
						std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
						std::cout << "Keyword: " << block << " requires 3 doubles" << std::endl;
						return eParseRetType::kEyeKeywordFormatError;
					}
				}
				rtPoint c(vec[0], vec[1], vec[2]);
				rtSphere sphere(c, vec[3]);
				sphere.m_materialIndex = m_objFileInfo->materials.size() - 1;
				m_objFileInfo->spheres.push_back(sphere);
			}
			else if (block == "light")
			{
				rtLight light;
				if (iss >> block)
				{
					light.setType(static_cast<eLightType>(std::stoi(block)));
				}
				switch (light.getType())
				{
				case eLightType::kPointLight:
				{
					std::vector<double> vec(6);
					for (int i = 0; i < 6; i++)
					{
						if (iss >> block)
						{
							vec[i] = std::stod(block);
						}
						else
						{
							std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
							std::cout << "Keyword: " << block << " requires 7 doubles" << std::endl;
							return eParseRetType::kEyeKeywordFormatError;
						}
					}
					light.setCenter(rtPoint(vec[0], vec[1], vec[2]));
					light.setColor(rtColor(vec[3], vec[4], vec[5]));
					light.setAttAttribute(1.0, 0.0, 0.0);
					m_objFileInfo->lights.push_back(light);
					break;
				}
				case eLightType::kDirectionalLight:
				{
					std::vector<double> vec(6);
					for (int i = 0; i < 6; i++)
					{
						if (iss >> block)
						{
							vec[i] = std::stod(block);
						}
						else
						{
							std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
							std::cout << "Keyword: " << block << " requires 7 doubles" << std::endl;
							return eParseRetType::kEyeKeywordFormatError;
						}
					}
					light.setVector3(rtVector3(vec[0], vec[1], vec[2]));
					light.setColor(rtColor(vec[3], vec[4], vec[5]));
					light.setAttAttribute(1.0, 0.0, 0.0);
					m_objFileInfo->lights.push_back(light);
					break;
				}
				case eLightType::kSpotlight:
				{
					std::vector<double> vec(10);
					for (int i = 0; i < 10; i++)
					{
						if (iss >> block)
						{
							vec[i] = std::stod(block);
						}
						else
						{
							std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
							std::cout << "Keyword: " << block << " requires 7 doubles" << std::endl;
							return eParseRetType::kEyeKeywordFormatError;
						}
					}
					light.setCenter(rtPoint(vec[0], vec[1], vec[2]));
					light.setVector3(rtVector3(vec[3], vec[4], vec[5]));
					light.setTheta(vec[6]);
					light.setColor(rtColor(vec[7], vec[8], vec[9]));
					light.setAttAttribute(1.0, 0.0, 0.0);
					m_objFileInfo->lights.push_back(light);
					break;
				}
				case eLightType::kAttPointLight:
				{
					std::vector<double> vec(9);
					for (int i = 0; i < 9; i++)
					{
						if (iss >> block)
						{
							vec[i] = std::stod(block);
						}
						else
						{
							std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
							std::cout << "Keyword: " << block << " requires 10 doubles" << std::endl;
							return eParseRetType::kEyeKeywordFormatError;
						}
					}
					light.setCenter(rtPoint(vec[0], vec[1], vec[2]));
					light.setColor(rtColor(vec[3], vec[4], vec[5]));
					light.setAttAttribute(vec[6], vec[7], vec[8]);
					m_objFileInfo->lights.push_back(light);
					break;
				}
				case eLightType::kAttSpotlight:
				{
					std::vector<double> vec(13);
					for (int i = 0; i < 10; i++)
					{
						if (iss >> block)
						{
							vec[i] = std::stod(block);
						}
						else
						{
							std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
							std::cout << "Keyword: " << block << " requires 7 doubles" << std::endl;
							return eParseRetType::kEyeKeywordFormatError;
						}
					}
					light.setCenter(rtPoint(vec[0], vec[1], vec[2]));
					light.setVector3(rtVector3(vec[3], vec[4], vec[5]));
					light.setTheta(vec[6]);
					light.setColor(rtColor(vec[7], vec[8], vec[9]));
					light.setAttAttribute(vec[10], vec[11], vec[12]);
					m_objFileInfo->lights.push_back(light);
					break;
				}
				default:
					break;
				}
			}
			else if (block == "v")
			{
				std::vector<double> vec(3);
				for (int i = 0; i < 3; i++)
				{
					if (iss >> block)
					{
						vec[i] = std::stod(block);
					}
					else
					{
						std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
						std::cout << "Keyword: " << block << " requires 3 doubles" << std::endl;
						return eParseRetType::kEyeKeywordFormatError;
					}
				}
				rtPoint p(vec[0], vec[1], vec[2]);
				m_objFileInfo->verteices.push_back(p);
			}
			else if (block == "vn")
			{
				std::vector<double> vec(3);
				for (int i = 0; i < 3; i++)
				{
					if (iss >> block)
					{
						vec[i] = std::stod(block);
					}
					else
					{
						std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
						std::cout << "Keyword: " << block << " requires 3 doubles" << std::endl;
						return eParseRetType::kEyeKeywordFormatError;
					}
				}
				rtVector3 v(vec[0], vec[1], vec[2]);
				m_objFileInfo->vertexNormals.push_back(v);
			}
			else if (block == "vt")
			{
				std::vector<double> vec(2);
				for (int i = 0; i < 2; i++)
				{
					if (iss >> block)
					{
						vec[i] = std::stod(block);
					}
					else
					{
						std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
						std::cout << "Keyword: " << block << " requires 2 doubles" << std::endl;
						return eParseRetType::kEyeKeywordFormatError;
					}
				}
				rtVector2<double> v(vec[0], vec[1]);
				m_objFileInfo->vertexTextureCoordinates.push_back(v);
			}
			else if (block == "f")
			{
				std::vector<std::vector<int>> vec;
				for (int i = 0; i < 3; i++)
				{
					std::string index;
					int n = 0;
					std::vector<int> face;
					if (iss >> block)
					{
						for (int j = 0; j < block.size(); j++)
						{
							if (block[j] != '/')
							{
								index.push_back(block[j]);
							}
							else
							{
								n++;
								face.push_back(index.empty() ? -1 : std::stoi(index));
								index = "";
							}
						}
						face.push_back(std::stoi(index));
						vec.push_back(face);
					}
					else
					{
						std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
						std::cout << "Keyword: " << block << " requires 3 doubles" << std::endl;
						return eParseRetType::kEyeKeywordFormatError;
					}
				}
				m_objFileInfo->faces.push_back(vec);
				m_objFileInfo->faceMaterialIndexs.push_back(m_objFileInfo->materials.size() - 1);
			}
			else if (block == "texture")
			{
				rtMaterial texMtl;
				if (iss >> block)
				{
					texMtl.setTextureFile(block);
					if (m_objFileInfo->materials.empty())
					{
						texMtl.setMtlProperties(0, 0, 0, 1, 1, 1, 0.6, 0.8, 0.2, 10, 1, 1);
					}
					else
					{
						texMtl.setMtlProperties(0, 0, 0, 1, 1, 1, m_objFileInfo->materials.back().m_ka, m_objFileInfo->materials.back().m_kd, m_objFileInfo->materials.back().m_ks, m_objFileInfo->materials.back().m_falloff, m_objFileInfo->materials.back().m_alpha, m_objFileInfo->materials.back().m_eta);
					}
				}
				else
				{
					std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
					std::cout << "Keyword: " << block << " requires a string" << std::endl;
					return eParseRetType::kEyeKeywordFormatError;
				}
				m_objFileInfo->materials.push_back(texMtl);
			}
		}
	}

	inFile.close();
	
	if (!hasEye || !hasImgSize || !hasBkgColor || !hasUpDir || !hasUpDir || !hasVFov)
	{
		std::cout << "-----------FILE PARSE ERROR-------------" << std::endl;
		return eParseRetType::kMissingKeywords;
	}

	return eParseRetType::kSuccess;
}