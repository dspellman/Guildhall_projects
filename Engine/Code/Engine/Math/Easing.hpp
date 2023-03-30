#pragma once
#include "MathUtils.hpp"

typedef float (*pointFunction)(float);

float Identity(float t);
float SmoothStart2(float t);
float SmoothStart3(float t);
float SmoothStart4(float t);
float SmoothStart5(float t);
float SmoothStart6(float t);
						   
float SmoothStop2(float t) ;
float SmoothStop3(float t) ;
float SmoothStop4(float t) ;
float SmoothStop5(float t) ;
float SmoothStop6(float t) ;
						   
float SmoothStep3(float t) ;
float SmoothStep5(float t) ;
						   
float Hesitate3(float t);
float Hesitate5(float t);

float FunkyFunc(float t);

// pointFunction easingFunction[] = {
// 	Identity,
// 	SmoothStart2,
// 	SmoothStart3,
// 	SmoothStart4,
// 	SmoothStart5,
// 	SmoothStart6,
// 	SmoothStop2,
// 	SmoothStop3,
// 	SmoothStop4,
// 	SmoothStop5,
// 	SmoothStop6,
// 	SmoothStep3,
// 	SmoothStep5,
// 	Hesitate3,
// 	Hesitate5,
// 	FunkyFunc,
// };
// 
// std::string easingFunctionName[] = {
// 	"Identity",
// 	"SmoothStart2",
// 	"SmoothStart3",
// 	"SmoothStart4",
// 	"SmoothStart5",
// 	"SmoothStart6",
// 	"SmoothStop2",
// 	"SmoothStop3",
// 	"SmoothStop4",
// 	"SmoothStop5",
// 	"SmoothStop6",
// 	"SmoothStep3",
// 	"SmoothStep5",
// 	"Hesitate3",
// 	"Hesitate5",
// 	"FunkyFunc",
// };
// 
// constexpr int FUNCTION_COUNT = sizeof(easingFunction) / sizeof(easingFunction[0]);

extern pointFunction easingFunction[];
extern std::string easingFunctionName[];
extern int FUNCTION_COUNT;