#include "PpmFileReader.h"
#include "rtColor.h"
#include "rtVector.h"

ppmFileReader::ppmFileReader(const std::string & fileName)
{
	m_fileName = fileName;
}

void ppmFileReader::getTextureArray(std::vector<rtColor>& texture, rtVector2<int>& size)
{
	std::ifstream inFile;
	inFile.open(m_fileName);
	
	if (inFile.fail())
	{
		return;
	}

	std::string placeHolder1, placeHolder2, width, height;

	inFile >> placeHolder1 >> width >> height >> placeHolder2;
	size.m_x = std::stoi(width);
	size.m_y = std::stoi(height);

	for (int i = 0; i < size.m_x * size.m_y; i++)
	{
		std::string r, g, b;
		inFile >> r >> g >> b;
		rtColor pixel(std::stoi(r), std::stoi(g), std::stoi(b));
		texture.push_back(pixel);
	}

	inFile.close();

}
