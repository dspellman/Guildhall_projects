#pragma once
#include <string>
#include <vector>

// rgb color class
struct Rgba8
{
public:
	unsigned char r = 255; 
	unsigned char g = 255; 
	unsigned char b = 255; 
	unsigned char a = 255;

	static Rgba8 const WHITE;
	static Rgba8 const RED;
	static Rgba8 const GREEN;
	static Rgba8 const BLUE;
	static Rgba8 const YELLOW;
	static Rgba8 const GRAY;
	static Rgba8 const GOLD;
	static Rgba8 const PURPLE;
	static Rgba8 const BLACK;
	static Rgba8 const CYAN;
	static Rgba8 const BROWN;

	Rgba8();
	explicit Rgba8(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a = 255);
	~Rgba8();

	void GetAsFloats(float* colorAsFloats) const;
	void SetFromFloats(float* colorAsFloats);
	bool SetFromText( const char* text );
	std::string ToString();
	static Rgba8 ColorLerp(const Rgba8& start, const Rgba8& end, float t);
	Rgba8 dim(float fraction) const;
	bool operator==(const Rgba8& compare) const;
	bool operator!=(const Rgba8& compare) const;
};