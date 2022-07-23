#include<iostream>
#include "ObjFileReader.h"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "file name required" << std::endl;
		return 0;
	}
	auto fileReader = std::make_unique<ObjFileReader>();

	return 0;
}
