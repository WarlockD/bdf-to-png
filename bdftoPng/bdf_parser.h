#pragma once
#include "bdftopng.h"
#include <string>
#include <vector>
#include <bitset>

typedef unsigned char t_byte;
struct BDF_Box { int w; int h; int x; int y;  };
struct BDF_Size { int w; int h; };

struct BDF_Glyph 
{
	unsigned encoding;
	BDF_Box bbx;
	BDF_Size dwidth;
	BDF_Size swidth;
	std::vector<std::bitset<BDF_MAX_BITS_PER_ROW>> bmp;
	BDF_Glyph();
	~BDF_Glyph();
};
typedef std::vector<BDF_Glyph>::iterator BDF_Glyph_it;
class BDF_Info
{
protected:
	bool _valid;
	std::string _filename;
	std::string _font;
	int _point;
	BDF_Size _size;
	BDF_Box _bbox; 
	int _nchars;
	std::vector<BDF_Glyph> _glyphs;
public:
	BDF_Info(const char* filename=NULL);
	void LoadFile(const char* filename);
};
