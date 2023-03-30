#include "Rgba8.hpp"
#include "StringUtils.hpp"
#include <string>
#include <vector>
#include "../Math/MathUtils.hpp"

Rgba8 const Rgba8::WHITE = Rgba8(255, 255, 255, 255);
Rgba8 const Rgba8::RED   = Rgba8(255, 0, 0, 255);
Rgba8 const Rgba8::GREEN = Rgba8(0, 255, 0, 255);
Rgba8 const Rgba8::BLUE  = Rgba8(0, 0, 255, 255);
Rgba8 const Rgba8::YELLOW = Rgba8(255, 255, 0, 255);
Rgba8 const Rgba8::GRAY = Rgba8(127, 127, 127, 255);
Rgba8 const Rgba8::GOLD = Rgba8(252, 165, 4, 255);
Rgba8 const Rgba8::PURPLE = Rgba8(90, 16, 115, 255);
Rgba8 const Rgba8::BLACK = Rgba8(0, 0, 0);
Rgba8 const Rgba8::CYAN = Rgba8(0, 255, 255, 255);
Rgba8 const Rgba8::BROWN = Rgba8(98,52,18, 255);

Rgba8::Rgba8() : r(255), g(255), b(255), a(255)
{
	// this constructor is redundant since already set to white, but assignment said to make it default to white
}

Rgba8::Rgba8(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a)
	: r(_r), g(_g), b(_b), a(_a)
{

}

Rgba8::~Rgba8()
{

}

// normalize byte values to zero to one range floats
void Rgba8::GetAsFloats(float* colorAsFloats) const
{
	colorAsFloats[0] = static_cast<float>(r) / 256.0f;
	colorAsFloats[1] = static_cast<float>(g) / 256.0f;
	colorAsFloats[2] = static_cast<float>(b) / 256.0f;
	colorAsFloats[3] = static_cast<float>(a) / 256.0f;
}

void Rgba8::SetFromFloats(float* colorAsFloats)
{
	r = static_cast<unsigned char>(255.9f * colorAsFloats[0]);
	g = static_cast<unsigned char>(255.9f * colorAsFloats[1]);
	b = static_cast<unsigned char>(255.9f * colorAsFloats[2]);
	a = static_cast<unsigned char>(255.9f * colorAsFloats[3]);
}

bool Rgba8::SetFromText(const char* text)
{
	Strings strings = SplitStringOnDelimiter(text, ',');
	if (strings.size() == 3)
	{
		// TODO: test that the strings are numeric and in range
		r = static_cast<unsigned char>(atoi(strings[0].c_str()));
		g = static_cast<unsigned char>(atoi(strings[1].c_str()));
		b = static_cast<unsigned char>(atoi(strings[2].c_str()));
		a = 255;
		return true;
	}
	if (strings.size() == 4)
	{
		// TODO: test that the strings are numeric and in range
		r = static_cast<unsigned char>(atoi(strings[0].c_str()));
		g = static_cast<unsigned char>(atoi(strings[1].c_str()));
		b = static_cast<unsigned char>(atoi(strings[2].c_str()));
		a = static_cast<unsigned char>(atoi(strings[3].c_str()));
		return true;
	}
	return false;
}

std::string Rgba8::ToString()
{
	std::string content = "(";
	content += std::to_string(r); 
	content += ", ";
	content += std::to_string(b); 
	content += ", ";
	content += std::to_string(g); 
	content += ", ";
	content += std::to_string(a); 
	content += ")";
	return content;
}

Rgba8 Rgba8::ColorLerp(const Rgba8& start, const Rgba8& end, float t)
{
	Clamp(t, 0.0f, 1.0f);
	Rgba8 lerpedColor = Rgba8::WHITE;
	lerpedColor.r = static_cast<unsigned char>(static_cast<float>(start.r) * (1.0f - t) + static_cast<float>(end.r) * t);
	lerpedColor.g = static_cast<unsigned char>(static_cast<float>(start.g) * (1.0f - t) + static_cast<float>(end.g) * t);
	lerpedColor.b = static_cast<unsigned char>(static_cast<float>(start.b) * (1.0f - t) + static_cast<float>(end.b) * t);
	lerpedColor.a = static_cast<unsigned char>(static_cast<float>(start.a) * (1.0f - t) + static_cast<float>(end.a) * t);
	return lerpedColor;
}

Rgba8 Rgba8::dim(float fraction) const
{
	float values[4];
	GetAsFloats(values);
	values[0] *= fraction;
	values[1] *= fraction;
	values[2] *= fraction;
	Rgba8 dimmed = Rgba8::WHITE;
	dimmed.SetFromFloats(values);
	return dimmed;
}

bool Rgba8::operator!=(const Rgba8& compare) const
{
	if (r != compare.r)
	{
		return true;
	}	
	if (g != compare.g)
	{
		return true;
	}
	if (b != compare.b)
	{
		return true;
	}
	if (a != compare.a)
	{
		return true;
	}
	return false;
}

bool Rgba8::operator==(const Rgba8& compare) const
{
	if (r != compare.r)
	{
		return false;
	}
	if (g != compare.g)
	{
		return false;
	}
	if (b != compare.b)
	{
		return false;
	}
	if (a != compare.a)
	{
		return false;
	}
	return true;
}
