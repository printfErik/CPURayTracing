cmake_minimum_required (VERSION 3.8)

project ("CPURayTracing")

add_executable (CPURayTracing "CPURayTracing.cpp"  "FileReader.cpp" "FileReader.h" "ObjFileReader.cpp" "ObjFileReader.h" "rtVector.cpp" "rtVector.h" "rtColor.cpp" "rtColor.h" "rtMaterial.h" "rtSphere.cpp" "rtSphere.h" "rtPoint.cpp" "rtPoint.h" "rtLight.cpp" "rtLight.h")

