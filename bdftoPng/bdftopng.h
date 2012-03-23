#pragma once

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

typedef unsigned char t_byte;
typedef unsigned char* t_bytep;

enum BDF_ERROR {
	BDF_NO_ERROR=0,
	BDF_EOF,
	BDF_ERROR_FILE,
	BDF_ERROR_ALLOC,
	BDF_ERROR_PNG,
	BDF_ERROR_OPEN,
	BDF_ERROR_EOS,
	BDF_ERROR_WRONGPOS,
	BDF_ERROR_BADTOKEN
};

// Font size of 64 by 64.  Want to hold off on any memory allocs as I can
// change this if you really need fonts bigger and your not going to post scale them
#define BDF_MAX_BITS_PER_ROW sizeof(unsigned int) // 32 bit lengh
#define BDF_PNG_WIDTH 512
#define BDF_PNG_HEIGHT 512


enum BDF_PNG {
	BDF_PNG_GRAY	= 0, 	// 1 byte, Grayscale
	BDF_PNG_GRAYA,			// 2 bytes, Grayscale + Alpha
	BDF_PNG_RGB,			// 3 bytes, Red + Green + Blue
	BDF_PNG_RGBA			// 4 bytes, Red + Green + Blue + Alpha
};
#define BDF_PNG_COLOR_MODE_DEFAULT BDF_PNG_GRAYA