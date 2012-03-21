#include "bdf_parser.h"
#include "bdftopng.h"

#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG 1
#endif
#endif

#ifdef DEBUG
#define TEST_GLYPH 'A'
#endif
// This REALLLY REALLY isn't unicode complient
// Screw it though, considering I am reading 20 year old font files
// I doubt anyone will care.  
#define ON_FALSE_ERROR(exp,err) if(!(exp)) { error =err; goto finalise; }
#define ON_TRUE_ERROR(exp,err) if(exp) { error =err; goto finalise; }

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <assert.h>

#define isWhiteSpace(ch) (ch == ' ' || ch == '\t' || ch == '\r')
#define isUpper(ch) (ch >= 'A' && ch <= 'Z')
#define isLower(ch) (ch >= 'a' && ch <= 'z')
#define isLetter(ch) (isUpper(ch) || isLower(ch))
#define isNumber(ch) (ch >= '0' && ch <= '9')
#define isHex(ch) ((ch >= '0' && ch <='9') || (ch >='A' && ch <='F')|| (ch >='a' && ch <='f'))
#define isLetterOrNumber(ch) ( isLetter(ch) || isNumber(ch) )
#define eatWhiteSpace(str) while(isWhiteSpace(*str) && *str !='\0') { str++; }
// Eat till end of line and skip the end of line marker
#define eatTillEndOfLine(str) while(*str != '\n' && *str !='\0') { str++; }
// Simple charToHex, returns >15 if not valid.

// Token Structure atoi
// If a line is more than 16 bytes, then humm.
enum {
	BDF_FLAG_ENCODING  = (1 << 1),
	BDF_FLAG_SWIDTH = (1 << 2),
	BDF_FLAG_DWIDTH = (1 << 3),
	BDF_FLAG_BBX = (1 << 4),
	BDF_FLAG_ALL = BDF_FLAG_ENCODING | BDF_FLAG_SWIDTH | BDF_FLAG_DWIDTH | BDF_FLAG_BBX
};
enum {
	TOK_STARTFONT=0,
	TOK_FONT,
	TOK_SIZE,
	TOK_FONTBOUNDINGBOX,
	TOK_STARTPROPERTIES,
	TOK_ENDPROPERTIES,
	TOK_CHARS,
// Mask states
	TOK_STARTCHAR ,
	TOK_ENCODING ,
	TOK_SWIDTH ,
	TOK_DWIDTH,
	TOK_BBX,
//
	TOK_BITMAP,
	TOK_ENDCHAR,
	TOK_ENDFONT,
	// End of tokens, all error status commands here
	TOK_ERROR,
	TOK_DATA,
	TOK_SOF,
	TOK_EOF
};
typedef struct  {
	const char* str;
	int len;
	int tok;
} t_sTokString;


t_sTokString tok_strings[] = { 
	{"STARTFONT",sizeof("STARTFONT")-1, TOK_STARTFONT},
	{"FONT",sizeof("FONT")-1,TOK_FONT},
	{"SIZE",sizeof("STARTFONT"),TOK_SIZE},
	{"FONTBOUNDINGBOX",sizeof("FONTBOUNDINGBOX")-1,TOK_FONTBOUNDINGBOX},
	{"STARTPROPERTIES",sizeof("STARTPROPERTIES")-1,TOK_STARTPROPERTIES},
	{"ENDPROPERTIES",sizeof("ENDPROPERTIES")-1,TOK_ENDPROPERTIES},
	{"CHARS",sizeof("CHARS")-1,TOK_CHARS},
	{"STARTCHAR",sizeof("STARTCHAR")-1,TOK_STARTCHAR},
	{"ENCODING",sizeof("ENCODING")-1,TOK_ENCODING},
	{"SWIDTH",sizeof("SWIDTH")-1,TOK_SWIDTH},
	{"DWIDTH",sizeof("DWIDTH")-1,TOK_DWIDTH},
	{"BBX",sizeof("BBX")-1,TOK_BBX},
	{"BITMAP",sizeof("BITMAP")-1,TOK_BITMAP},
	{"ENDCHAR",sizeof("ENDCHAR")-1,TOK_ENDCHAR},
	{"ENDFONT",sizeof("ENDFONT")-1,TOK_ENDFONT},
	{NULL,0,TOK_ERROR},
};

static const t_sTokString tok_str_endchar = {"ENDCHAR",sizeof("ENDCHAR")-1,TOK_ENDCHAR};
static const char *hex2bin[]= { 
		"0000","0001","0010","0011","0100","0101","0110","0111",
		"1000","1001","1010","1011","1100","1101","1110","1111"
	};

// Its funny.  I started out wanting to use gotos on this but ended up
// using a bunch of for loops?  I wonder witch is more readable?
// The freebsd/netbsd version of strok_r is just funky though
// Some reason this looks sexy too.  merrow
static char* strok_line(char**next) {
	char *s,*tok; 
	if (next == NULL || *next == NULL) return (NULL);

	// Go to the first line mark
	for(s = *next;(*s != '\r' && *s != '\n' && *s != '\0');s++);
	// We mark it then, this is our new line and eat
	// eat any extra lines or marks or whitepsace
	for(*s++ = '\0';(*s == '\r' || *s == '\n'|| *s == ' ' || *s == '\t') && *s != '\0';s++);
	// Last line? set last nill and return the line
	tok = *next; 
	*next = *s == '\0' ? NULL : s;
	return tok; 
}

static char* strtok_r(char *s, const char *delim, char **last)
{
	char *spanp, *tok;
	int c, sc;
	if (s == NULL && (s = *last) == NULL) return (NULL);
cont:
	c = *s++;
	for (spanp = (char *)delim; (sc = *spanp++) != 0;) 
	{
		if (c == sc)
			goto cont;
	}
	if (c == 0) {  
		*last = NULL;
		return (NULL);
	}
	tok = s - 1;
	
	for (;;) 
	{
		c = *s++;
		spanp = (char *)delim;
		do {
			if ((sc = *spanp++) == c) 
			{
				if (c == 0)
					s = NULL;
				else
					s[-1] = '\0';
				*last = s;
				return (tok);
			}
		} while (sc != 0);
	}
/* NOTREACHED */
}

static int getToken(char* s) {
	int len = strlen(s);
	t_sTokString* t=tok_strings;
	for(;t->tok != TOK_ERROR;t++) 
		if(len == t->len)
			if(!strcmp(s,t->str))
				return t->tok;
	
	return TOK_ERROR;
}


int OpenBDF(const char* filename, t_sBDFStream* stream) {
	
	int error = BDF_NO_ERROR;
	int size = 0;
	FILE *f = NULL;
	char* data=NULL;
	ON_FALSE_ERROR(f = fopen(filename, "r"),BDF_ERROR_FILE);
	memset(stream,0,sizeof(t_sBDFStream));
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);

	data = (char *)malloc(size+1);
	ON_TRUE_ERROR(size != fread(data, sizeof(char), size, f),BDF_ERROR_FILE);
	stream->len = size;
	stream->start = data;
	stream->pos = NULL;
	stream->next = NULL;
	// Null as eveything went fine and we don't want to free the data
	data = NULL;
	
finalise:
	if(f)  fclose(f);
	if(data) free(data);
	return error;
}



static void printGlyph(unsigned char* bmp, unsigned encoding, int width, int height) {
	char buff[512];
	int x,y;
	printf("-------GLYPH------\n");
	printf("Glyph: %c (% 2w, % 2h)",encoding,width,height);
	for(y=0; y<height;y++) 
	{
		for(x=0;x < width;x++)
			buff[x] = bmp[height * y + x] != 0 ? '*' : ' ';
		buff[x] = '\0';
		printf("(%02d) : %s\n",y,buff);
	}
}



int ParseBDF(t_sBDFStream* stream,t_BDFInfo* info)
{ 
	char buffer[1024];
	unsigned encoding;
	unsigned flags;
	struct { int w; int h; } swidth;
	struct { int w; int h; } dwidth;
	struct { int w; int h; int x; int y; } bbx;
	int itemp=0;
	int state =0;
	int linenumber =0;
	int bpos =0;
	int tok = TOK_SOF;
	int i=0;
	int error = BDF_ERROR_FILE;
	char* last=NULL;
	char* pos = NULL;
	char* line = NULL;
	// Error out if we are not at the end of a stream or not at the begining
	// of the next line to read
	
	ON_FALSE_ERROR(stream->start, BDF_ERROR_EOS);
	memset(info,0,sizeof(t_BDFInfo));
	last = stream->start;
	while(line = strok_line(&last)) 
	{
		char* next=NULL;
		linenumber++;
		pos = strtok_r(line," \t",&next);
#if 0
		if(!pos)
			printf("(%0000d) %s : %s\n",linenumber,"BADTOKEN",line);
		else 
		{
			tok=getToken(pos);
			printf("(%0000d) %s : %s\n",linenumber,pos,next);
		}
#else
		
#endif
		
		assert(pos);
		tok=getToken(pos);

		if(tok == TOK_ERROR) continue;
		pos = NULL;
		// Some of these fields I am just going to ignore 
		switch(tok) {
			case TOK_STARTFONT:
				break; 
			case TOK_FONT: 
				break; 
			case TOK_SIZE:
				i=sscanf(next,"%d %d %d",&info->size.p,&info->size.w,&info->size.h);
				assert(i==3);
				break;
			case TOK_FONTBOUNDINGBOX:
				i=sscanf(next,"%d %d %d %d",&info->bbox.w,&info->bbox.h,&info->bbox.x,&info->bbox.y);
				assert(i==4);
				break;
			case TOK_CHARS:
				i=sscanf(next,"%d",&info->chars);
				assert(i==1);
				break;
			case TOK_STARTCHAR:
				state = TOK_STARTCHAR;
				flags = 0;
				break;
			case TOK_ENCODING:
				if(state != TOK_STARTCHAR) break;
				i=sscanf(next,"%d",&encoding);
				assert(i==1);
				flags |=BDF_FLAG_ENCODING;
				break;
			case TOK_SWIDTH:
				if(state != TOK_STARTCHAR) break;
				i=sscanf(next,"%d %d",&swidth.w,&swidth.h );
				assert(i==2);
				flags |=BDF_FLAG_SWIDTH;
				break;
			case TOK_DWIDTH:
				if(state != TOK_STARTCHAR) break;
				i=sscanf(next,"%d %d",&dwidth.w,&dwidth.h );
				assert(i==2);
				flags |=BDF_FLAG_DWIDTH;
				break;
			case TOK_BBX:
				if(state != TOK_STARTCHAR) break;
				i=sscanf(next,"%d %d %d %d",&bbx.w,&bbx.h,&bbx.x,&bbx.y);
				assert(i==4);
				flags |=BDF_FLAG_BBX;
				break;
			case TOK_BITMAP:
				if(state != TOK_STARTCHAR && flags != BDF_FLAG_ALL) 
				{
					printf("(%05d): Error at Bitmap, not all flags set before bitmap\n",linenumber);
					assert(0); exit(EXIT_FAILURE);
				} else 
				{
					unsigned char* bmp = NULL;
					int bline=0;
					bmp = (unsigned char*)malloc(sizeof(unsigned char) * bbx.w * bbx.h);
					ON_FALSE_ERROR(bmp,BDF_ERROR_ALLOC);
					while(line = strok_line(&last)) 
					{
						char *p = buffer;
						char c;
						int len = strlen(line);
						int hex;
						linenumber++;
						// Check end of bits
							if(len == tok_str_endchar.len)
							if(!strncmp(line,tok_str_endchar.str,tok_str_endchar.len)) 
								break;
							memset(buffer,' ',len*4); 
							buffer[len*4] = 0;
							for(i=0; i<len; i++) 
							{
								c = line[i];
								if(c >= '0' || c <= '9') hex = c - '0';
								else if(c >= 'a' || c <= 'f') hex =  c - 'a';
								else if(c >= 'A' || c <= 'B') hex =  c - 'A';
								else {
									printf("HEX PARSE ERROR\n");
									assert(0); exit(EXIT_FAILURE);
								}

							//	sprintf(buffer, "0x%c", line[i]); /* get one character from hexadecimal strings */
								//d = 
								//hex = hexval(line[i]);
								memcpy(bmp+(bline*bbx.w) + (i*4),hex2bin[hex],4);
								//strcpy((char*)(buffer + (i*4)),hex2bin[hex]);
							}
							for(i=0;i<bbx.w;i++) bmp[(bline * bbx.h) + i] = buffer[i] -'0';
							bline++;
					}

					if(encoding == 'A')
						printGlyph(bmp,encoding,bbx.w,bbx.h);
					printf("BITMAP: %d \n",encoding);
					if(encoding == (unsigned)'A')
					{
						printGlyph(bmp,encoding,bbx.w,bbx.h);
						assert(0);
					}

					info->glyphs[encoding].bmp = bmp;
					info->glyphs[encoding].offx = bbx.x;
					info->glyphs[encoding].offy = bbx.y;
					info->glyphs[encoding].h = bbx.h;
					info->glyphs[encoding].w = bbx.w;
					// Cleanup
					state = TOK_STARTFONT;
					bmp = NULL;
					memset(&bbx,0,sizeof(bbx));
				}
				break;
			case TOK_ENDFONT:
				error = BDF_NO_ERROR;
				goto finalise;
			default:
				break;
		}
	}

finalise:
	return error;
}
int CloseBDF(const char* filename, t_sBDFStream* stream) { return 0;}
