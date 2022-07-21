#include "ObjFileReader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

ObjFileReader::ObjFileReader()
{
	m_objFileInfo = std::make_unique<ObjFileInfo>();
}

eParseRetType ObjFileReader::parseFile()
{
	_ASSERT(!m_objFileInfo);
	std::ifstream inFile;
	inFile.open(m_fileName);

	if (inFile.fail())
	{
		std::cout << "Can't find this file: " << m_fileName << std::endl;
		return eParseRetType::kFileNotExists;
	}

	std::string line;
	while (std::getline(inFile, line))
	{
		std::istringstream iss(line);
		std::string block;
		while (iss >> block)
		{
			if (block == "eye")
			{
				std::vector<float> v3(3);
				for (int i = 0; i < 3; i++)
				{
					if (iss >> block)
					{
						v3[i] = std::stof(block);
					}
					else
					{
						std::cout << "-----------FILE PARSE ERROR-------------"<< std::endl;
						std::cout << "Keyword: " << block << " requires 3 floats" << m_fileName << std::endl;
						return eParseRetType::kEyeKeywordFormatError;
					}
				}
				rtVector3 vx();
				m_objFileInfo->eye = vx;
			}

		}
	}
}