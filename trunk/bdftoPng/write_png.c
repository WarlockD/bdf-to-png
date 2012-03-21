#include "write_png.h"

#include <png.h>
#include <stddef.h>
#include <stdio.h>
#include <malloc.h>



#define INIT_PNG(p_png,p_info) \
	p_png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);\
	if(!p_png) \
		{ return BDF_ERROR_PNG; }\
	p_info = png_create_info_struct(p_png);\
	if(!p_info) \
		{ png_destroy_write_struct(&p_png, NULL);  return BDF_ERROR_PNG; }\
	if (setjmp(png_jmpbuf(p_png))) \
		{ png_destroy_write_struct(&p_png, &p_info);  return BDF_ERROR_PNG; }



int WriteCharInBitmap(t_s1bitBitmap*buffer, int buffer_x, int buffer_y, unsigned char* data, int width, int height)
{
	int x=0,y=0,i=0;
	for(y=0;y  < height; y++)
	{
		unsigned char* gRow = buffer->grid[buffer_y +y];
		for(x=0,i=buffer_x; x< width; x++) 
		{
			unsigned char c = data[y * height + x];
			gRow[i++] = c;	// Grey
			gRow[i++] = c; // Alpha
		}
	}
	return BDF_NO_ERROR;
}

#define ON_ERROR_GOTO(exp,err) if(!(exp)) { error =err; goto finalise; }
int WritePNG_GA1(const char* filename, t_s1bitBitmap* buffer) 
{
	int x=0,y=0,i=0,error = BDF_NO_ERROR;
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	//png_colorp palette;
	png_bytep row;
	
	ON_ERROR_GOTO(fp = fopen(filename, "wb"),BDF_ERROR_FILE)
	ON_ERROR_GOTO(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL),BDF_ERROR_PNG)
	ON_ERROR_GOTO(info_ptr = png_create_info_struct(png_ptr),BDF_ERROR_PNG)
	ON_ERROR_GOTO(!setjmp(png_jmpbuf(png_ptr)),BDF_ERROR_PNG)
	ON_ERROR_GOTO(row = (png_bytep) malloc(BDF_PNG_WIDTH * sizeof(png_byte)*4),BDF_ERROR_PNG)

	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, BDF_PNG_WIDTH, BDF_PNG_HEIGHT, 8, PNG_COLOR_TYPE_GA,
				PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	//palette = (png_colorp)png_malloc(png_ptr, PNG_MAX_PALETTE_LENGTH * sizeof (png_color));
	//png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);
	png_write_info(png_ptr, info_ptr);
	png_set_packing(png_ptr);

	// I am sure there is an easyer way but this is just a test
	for(y=0; y < BDF_PNG_HEIGHT; y++) 
	{
		for(x=0,i=0; x < BDF_PNG_WIDTH;x++) 
			if(buffer->grid[y][x]) 
				{ row[i++] = 0xFF; row[i++] = 0xFF; row[i++] = 0xFF; row[i++] = 0xFF; }
			else
				{ row[i++] = 0x00; row[i++] = 0x00; row[i++] = 0x00; row[i++] = 0x00; }
		png_write_row(png_ptr, row);	
	}
	png_write_end(png_ptr, NULL);

	// Sometimes goto isn't as bad as you think, atleast for error functions and parsers
finalise:
   if (fp)			fclose(fp);
   if (info_ptr)	png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
   if (png_ptr)		png_destroy_write_struct(&png_ptr, NULL);
   if (row)			free(row);
   return error;
}