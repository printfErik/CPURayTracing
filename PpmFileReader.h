#pragma once
#include <fstream>
#include <vector>
#include <string>

class rtColor;
class PpmFileReader
{
public:
	PpmFileReader(const std::string& fileName);
	void getTextureArray(std::vector<rtColor>& texture, rtVector2& size);

private:
	std::string m_fileName;
};