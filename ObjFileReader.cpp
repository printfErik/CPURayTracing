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
		std::string keyword;
		while (iss >> keyword)
		{
			if (std::algorithm::firstToken == "")
		}
	}
}