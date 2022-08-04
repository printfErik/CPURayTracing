#include<iostream>
#include "ObjFileReader.h"
#include "rayTracer.h"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "file name required" << std::endl;
		return 0;
	}

	auto rayTracerApp = std::make_unique<rayTracer>();

	rayTracerApp->Init(argv[1]);
	rayTracerApp->ComputeUV();
	rayTracerApp->ComputeAspectRatioAndRenderPlane();

	
	return 0;
}
