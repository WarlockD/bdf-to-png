#ifndef _BDF_PARSER_H_
#define _BDF_PARSER_H_

#include "bdftopng.h"



typedef struct {
	struct { int p; int w; int h; } size;
	struct { int w; int h; int x; int y; } bbox; 
	int chars;
	struct {
		int w;
		int h;
		int offx;
		int offy;
		unsigned char* bmp;
	} glyphs[256];
} t_BDFInfo;

// Cheap stream used to get a token 
typedef struct {
	char* pos;
	char* next;
	char* start;
	int len;
} t_sBDFStream;


int OpenBDF(const char* filename, t_sBDFStream* stream);
int ParseBDF(t_sBDFStream* stream,t_BDFInfo* info);
int CloseBDF(const char* filename, t_sBDFStream* stream);

#endif