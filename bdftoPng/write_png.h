#ifndef _WRITE_PNG_
#define _WRITE_PNG_

#include "bdftopng.h"

typedef struct {
	unsigned char grid[BDF_PNG_HEIGHT][BDF_PNG_WIDTH*2];
} t_s1bitBitmap;

#ifdef __cplusplus
extern "C" {
#endif

	// Internaly the font is saved as a PNG_COLOR_TYPE_GA
	// This is grayscale + alpha.  So b[0] = grey; b[1] = alpha
	// We save bits and it all gets converted when it loads anyway.  We can also
	// do anti alising on these as well.
	// Technicaly this is also pre-multiplyed alpha as well.

	 int WritePNG_GA(const char* filename, t_s1bitBitmap* buffer);
	 int WriteCharInBitmap(t_s1bitBitmap*buffer, int buffer_x, int buffer_y, unsigned char* data, int width, int height); 

#ifdef __cplusplus
}
#endif


#endif