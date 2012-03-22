#ifndef _BDF_PARSER_H_
#define _BDF_PARSER_H_
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
	std::vector<std::bitset<32>> bmp;
	BDF_Glyph();
	~BDF_Glyph();
};
class BDF_Info
{
private:
	std::string _font;
	int _point;
	BDF_Size _size;
	BDF_Box _bbox; 
	int _nchars;
	std::vector<BDF_Glyph> _glyphs;
public:
	BDF_Info();
	void LoadFile(const char* filename);
};


#endif