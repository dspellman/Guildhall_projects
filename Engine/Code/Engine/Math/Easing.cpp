#include "Engine/Math/Easing.hpp"

float Identity(float t) { return t; }
float SmoothStart2(float t) { return t * t; }
float SmoothStart3(float t) { return t * t * t; }
float SmoothStart4(float t) { return t * t * t * t; }
float SmoothStart5(float t) { return t * t * t * t * t; }
float SmoothStart6(float t) { return t * t * t * t * t * t; }

float SmoothStop2(float t) { float s = 1.0f - t; return 1.0f - s * s; }
float SmoothStop3(float t) { float s = 1.0f - t; return 1.0f - s * s * s; }
float SmoothStop4(float t) { float s = 1.0f - t; return 1.0f - s * s * s * s; }
float SmoothStop5(float t) { float s = 1.0f - t; return 1.0f - s * s * s * s * s; }
float SmoothStop6(float t) { float s = 1.0f - t; return 1.0f - s * s * s * s * s * s; }

float SmoothStep3(float t) { return ComputeCubicBezier1D(0.0f, 0.0f, 1.0f, 1.0f, t); }
float SmoothStep5(float t) { return ComputeQuinticBezier1D(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, t); }

float Hesitate3(float t) { return ComputeCubicBezier1D(0.0f, 1.0f, 0.0f, 1.0f, t); }
float Hesitate5(float t) { return ComputeQuinticBezier1D(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, t); }

float FunkyFunc(float t) { return SmoothStart2(Hesitate5(sinf(t * (float)Pi() * 6.0f) * 0.2f + t)); }

pointFunction easingFunction[] = {
	Identity,
	SmoothStart2,
	SmoothStart3,
	SmoothStart4,
	SmoothStart5,
	SmoothStart6,
	SmoothStop2,
	SmoothStop3,
	SmoothStop4,
	SmoothStop5,
	SmoothStop6,
	SmoothStep3,
	SmoothStep5,
	Hesitate3,
	Hesitate5,
	FunkyFunc,
};

std::string easingFunctionName[] = {
	"Identity",
	"SmoothStart2",
	"SmoothStart3",
	"SmoothStart4",
	"SmoothStart5",
	"SmoothStart6",
	"SmoothStop2",
	"SmoothStop3",
	"SmoothStop4",
	"SmoothStop5",
	"SmoothStop6",
	"SmoothStep3",
	"SmoothStep5",
	"Hesitate3",
	"Hesitate5",
	"FunkyFunc",
};

int FUNCTION_COUNT = sizeof(easingFunction) / sizeof(easingFunction[0]);