#pragma once
#include <fstream>
#include <vector>
#include <string>
#include "rtVector.h"
#include "rtColor.h"

class ppmFileReader
{
public:
	ppmFileReader(const std::string& fileName);
	void getTextureArray(std::vector<rtColor>& texture, rtVector2<int>& size);

private:
	std::string m_fileName;
};