#ifndef _BDFTOPNG_H_
#define _BDFTOPNG_H_

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

enum {
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
#define MAX_1BIT_FONT_DATA_SIZE ((64 * 64) / 8)
#define BDF_PNG_WIDTH 512
#define BDF_PNG_HEIGHT 512






#endif