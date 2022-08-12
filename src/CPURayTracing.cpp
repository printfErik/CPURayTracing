#include<iostream>
#include "ObjFileReader.h"
#include "rayTracer.h"

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cout << "file name required" << std::endl;
		return 0;
	}

	auto rayTracerApp = std::make_unique<rayTracer>();

	rayTracerApp->Init(argv[1]);
	rayTracerApp->ReadTextureFiles();
	rayTracerApp->ComputeUV();
	rayTracerApp->ComputeAspectRatioAndRenderPlane();
	rayTracerApp->InitPixelArray();
	rayTracerApp->CreatePixelIndexTo3DPointMap();
	rayTracerApp->CreatePixelIndexToRayMap();
	rayTracerApp->ComputePixelColor();
	rayTracerApp->OutputFinalImage(argv[2]);

	return 0;
}
