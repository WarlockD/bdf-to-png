#ifndef _BDF_PARSER_H_
#define _BDF_PARSER_H_

#include "bdftopng.h"
typedef unsigned char t_byte;
typedef struct { int w; int h; int x; int y; } t_BDFBox;
typedef struct { int w; int h;  } t_BDFSize;
typedef struct {
	t_BDFBox bbx;
	t_BDFSize dwidth;
	t_BDFSize swidth;
	t_byte* bmp;
} t_BDFGlyph;

typedef struct {
	char font[128];
	int point;
	t_BDFSize size;
	t_BDFBox bbox; 
	int nchar;
	t_BDFGlyph glyphs[256];
} t_BDFInfo;


int OpenBDF(const char* filename, t_BDFInfo* info);
int CloseBDF(t_BDFInfo* stream);

#endif