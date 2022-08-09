#include "PpmFileReader.h"
#include "rtColor.h"


PpmFileReader::PpmFileReader(const std::string& fileName)
{
	m_fileName = fileName;
}

void PpmFileReader::getTextureArray(std::vector<rtColor>& texture, int& width, int& height)
{
	std::ifstream inFile;
	inFile.open(m_fileName);
	std::string str;

	int placeHolder1, placeHolder2;

	inFile >> placeHolder1 >> width >> height >> placeHolder2;

	for (int i = 0; i < height * width; i++)
	{
		std::string r, g, b;
		inFile >> r >> g >> b;
		rtColor pixel(std::stoi(r), std::stoi(g), std::stoi(b));
		texture.push_back(pixel);
	}

	inFile.close();

}
