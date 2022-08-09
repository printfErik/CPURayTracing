#include "PpmFileReader.h"
#include "rtColor.h"
#include "rtVector.h"

PpmFileReader::PpmFileReader(const std::string& fileName)
{
	m_fileName = fileName;
}

void PpmFileReader::getTextureArray(std::vector<rtColor>& texture, rtVector2& size)
{
	std::ifstream inFile;
	inFile.open(m_fileName);
	std::string str;

	int placeHolder1, placeHolder2;

	inFile >> placeHolder1 >> size.m_x >> size.m_y >> placeHolder2;

	for (int i = 0; i < (int)size.m_x * (int)size.m_y; i++)
	{
		std::string r, g, b;
		inFile >> r >> g >> b;
		rtColor pixel(std::stoi(r), std::stoi(g), std::stoi(b));
		texture.push_back(pixel);
	}

	inFile.close();

}
