
#include "utils.h"
#include "bdf_parser.h"
#include "bdftopng.h"
#include "utils.h"
#include "bdftopng.h"


#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG 1
#endif
#endif

#include <string>
#include <fstream>
#include <iostream> 
#include <sstream> 

#include <exception>
using namespace std;



class bdf_exception: public std::exception
{
	string _msg;
	int _linenumber;
public:
	bdf_exception(const char* msg, int linenumber) 
	{
		_linenumber = linenumber;
		_msg = sutil::format("Line(%04d): %s\n",linenumber,msg);
	}
	virtual const char* what() const throw() { return _msg.c_str();}
} ;


#define ON_TRUE_ERROR(exp,msg) if(exp) { throw bdf_exception(msg,linenumber);  }
#define ON_FALSE_ERROR(exp,msg) if(!(exp)) {  throw bdf_exception(msg,linenumber);  }
#define ON_NULL_ERROR(exp,msg) if(!(exp)) { _CrtDumpMemoryLeaks(); throw bdf_exception(msg,linenumber);   }

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
#define Hex2Int(c) ((int)((int)c <='9' ? (int)c - '0' : (int)c <='F' ? (int)c -'A' + 10 : (int)c - 'a' + 10))
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
	{"ENDCHAR",sizeof("ENDCHAR")-1,TOK_ENDCHAR},
	{"STARTFONT",sizeof("STARTFONT")-1, TOK_STARTFONT},
	{"FONT",sizeof("FONT")-1,TOK_FONT},
	{"SIZE",sizeof("STARTFONT"),TOK_SIZE},
	{"FONTBOUNDINGBOX",sizeof("FONTBOUNDINGBOX")-1,TOK_FONTBOUNDINGBOX},
	//{"STARTPROPERTIES",sizeof("STARTPROPERTIES")-1,TOK_STARTPROPERTIES},
	//{"ENDPROPERTIES",sizeof("ENDPROPERTIES")-1,TOK_ENDPROPERTIES},
	{"CHARS",sizeof("CHARS")-1,TOK_CHARS},
	{"STARTCHAR",sizeof("STARTCHAR")-1,TOK_STARTCHAR},
	{"ENCODING",sizeof("ENCODING")-1,TOK_ENCODING},
	{"SWIDTH",sizeof("SWIDTH")-1,TOK_SWIDTH},
	{"DWIDTH",sizeof("DWIDTH")-1,TOK_DWIDTH},
	{"BBX",sizeof("BBX")-1,TOK_BBX},
	{"BITMAP",sizeof("BITMAP")-1,TOK_BITMAP},
	{"ENDFONT",sizeof("ENDFONT")-1,TOK_ENDFONT},
	{NULL,0,TOK_ERROR},
};
#define TOK_ENDCHAR_STR tok_strings[0]

static const char *hex2bin[]= { 
		"0000","0001","0010","0011","0100","0101","0110","0111",
		"1000","1001","1010","1011","1100","1101","1110","1111"
	};

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

static int getToken(const string& s) {
	t_sTokString* t=tok_strings;
	for(;t->tok != TOK_ERROR;t++) 
		if(!s.compare(0,t->len,t->str,t->len))
			return t->tok;
	return TOK_ERROR;
}

BDF_Glyph::BDF_Glyph()
{
	encoding = 0;
	bbx.x = bbx.y = bbx.h = bbx.w = 0; 
	dwidth.h = dwidth.w = 0;
	swidth.h = swidth.w = 0;
	bmp.reserve(160);
}
BDF_Glyph::~BDF_Glyph() {}

BDF_Info::BDF_Info() {
	_point =0;
	_size.h = _size.w = 0;
	_bbox.x = _bbox.y = _bbox.h = _bbox.w = 0; 
	_font = "";
	_glyphs.reserve(256);
}
void BDF_Info::LoadFile(const char* filename) 
{
	//const char* err_msg = NULL;
	// File vars
	ifstream f;
	string line;
	string temp;
	// Token State
	int state =0; int linenumber =0; int bl=0;
	unsigned flags; unsigned encoding;
	BDF_Glyph g; // temp gliphy
	// String buffers and line data
	try
	{
		f.open(filename,ios::in);
		ON_FALSE_ERROR(f.is_open(),"Could not open file");
		while(getline(f,line))
		{
			// Check if we are in a BITMAP state
			if(state == TOK_BITMAP) 
			{
				string bits;
				std::bitset<32> bitline;
				// Check end of bits
				if(line.length() == TOK_ENDCHAR_STR.len)
					if(!line.compare(0,TOK_ENDCHAR_STR.len, TOK_ENDCHAR_STR.str,TOK_ENDCHAR_STR.len))
					{
#if DEBUG
						cout << "-------------" << endl;
						cout << "Char(" << encoding << ")" << endl;
						for(int i=0;i < g.bbx.h;i++)
							cout << g.bmp[i].to_string() << endl;

#endif
						_glyphs.push_back(g);
						state = TOK_STARTFONT;
						continue;
					}
					// This is SLOW but 100% it works.  So going to leave it till the rest of it is done.
				for(unsigned i=0;i<line.length();i++) 
				{
					char c = line[i];
					if(!isHex(c)) break;
					int hex = 0xF & Hex2Int(c);
					bits+=hex2bin[hex];
				}
				for(int i=0;i<g.bbx.w;i++) 
					bitline[i] = bits[i] == '1' ? true : false;
				g.bmp.push_back(bitline);
				continue;
			}
			stringstream line_stream(line);
			string tokstr;
			line_stream >> tokstr;

			int tok = getToken(tokstr);	
			// If token not supported or error, continue
			if(tok == TOK_ERROR) continue;

			switch(tok) {
				case TOK_STARTFONT: state=TOK_STARTFONT; break; 
				case TOK_FONT: 
					// Get end of stream
					getline(line_stream,_font);
					break; 
				case TOK_SIZE:
					line_stream >> _point >> _size.w >> _size.h;
					break;
				case TOK_FONTBOUNDINGBOX:
					line_stream >> _bbox.w >> _bbox.h >> _bbox.x >> _bbox.y;
					break;
				case TOK_CHARS:
					line_stream >> _nchars; 
					break;
				case TOK_STARTCHAR:
					ON_FALSE_ERROR(state == TOK_STARTFONT || flags, "Header not Read")
					state = TOK_STARTCHAR;
					flags = 0; encoding = 0;
					break;
				case TOK_ENCODING:
					ON_FALSE_ERROR(state == TOK_STARTCHAR, "Not in a STARTCHAR")
					line_stream >> encoding;
					flags |=BDF_FLAG_ENCODING;
					break;
				case TOK_SWIDTH:
					ON_FALSE_ERROR(state == TOK_STARTCHAR, "Not in a STARTCHAR")
					line_stream >> g.swidth.w >> g.swidth.h;
					flags |=BDF_FLAG_SWIDTH;
					break;
				case TOK_DWIDTH:
					ON_FALSE_ERROR(state == TOK_STARTCHAR, "Not in a STARTCHAR")
					line_stream >> g.dwidth.w >> g.dwidth.h;
					flags |=BDF_FLAG_DWIDTH;
					break;
				case TOK_BBX:
					ON_FALSE_ERROR(state == TOK_STARTCHAR, "Not in a STARTCHAR")
					line_stream >>  g.bbx.w >> g.bbx.h >> g.bbx.x >> g.bbx.y;
					flags |=BDF_FLAG_BBX;
					break;
				case TOK_BITMAP:
					ON_FALSE_ERROR(state == TOK_STARTCHAR, "Not in a STARTCHAR")
					ON_FALSE_ERROR(flags == BDF_FLAG_ALL, "STARTCHAR Header not fully set before BITMAP") 
					g.bmp.clear();
					state = TOK_BITMAP;
					bl=0; // Current bitmap line
					break;
				case TOK_ENDFONT:
					return;
				default:
					break;
			}
		}
	// Once done we don't need anymore
	} catch(exception &e) 
	{
		printf(e.what());
		exit(-1); // find code
	}

}








/*
int OpenBDF(const char* filename,t_BDFInfo* info)
{ 
	size_t test;
	
	

	// Error out if we are not at the end of a stream or not at the begining
	// of the next line to read
	
	memset(info,0,sizeof(t_BDFInfo)); // Clear the data
	last = fdata;
	while(line = strok_line(&last)) 
	{
		int i,tok;
		char *stok=NULL,*next=NULL;
		linenumber++;
		// Check if we are in a BITMAP state
		if(state == TOK_BITMAP) 
		{
			char c;
			int len,hex;
			len = strlen(line);
			// Check end of bits
			if(len == TOK_ENDCHAR_STR.len)
				if(!strncmp(line,TOK_ENDCHAR_STR.str,TOK_ENDCHAR_STR.len))
				{
					memcpy(&info->glyphs[encoding],&g,sizeof(t_BDFGlyph));
					state = TOK_STARTFONT;
					continue;
				}
			for(i=0;i<len;i++) 
			{
				c = line[i];
				hex = Hex2Int(c);
				memcpy(buffer + (i*4),hex2bin + c,sizeof(char) * 4);
			}
			for(i=0;i<g.bbx.w;i++) 
				g.bmp[(bl*g.bbx.h) + i] = buffer[i];
			bl++;
			continue;
		}

		// Seperate the first part of the token with the next
		stok = strtok_r(line," \t",&next); assert(stok);
		tok = getToken(stok);
		// If token not supported or error, continue
		if(tok == TOK_ERROR) continue;

		switch(tok) {
			case TOK_STARTFONT: state=TOK_STARTFONT; break; 
			case TOK_FONT: 
				strncpy(info->font,next,sizeof(info->font)-1);
				break; 
			case TOK_SIZE:
				i=sscanf(next,"%d %d %d",&info->point,&info->size.w,&info->size.h);
				assert(i==3);
				break;
			case TOK_FONTBOUNDINGBOX:
				i=sscanf(next,"%d %d %d %d",&info->bbox.w,&info->bbox.h,&info->bbox.x,&info->bbox.y);
				assert(i==4);
				break;
			case TOK_CHARS:
				i=sscanf(next,"%d",&info->nchar);
				assert(i==1);
				break;
			case TOK_STARTCHAR:
				ON_FALSE_ERROR(state == TOK_STARTFONT || flags, "Header not Read")
				state = TOK_STARTCHAR;
				memset(&g,0,sizeof(t_BDFGlyph));
				flags = 0; encoding = 0;
				break;
			case TOK_ENCODING:
				ON_FALSE_ERROR(state == TOK_STARTCHAR, "Not in a STARTCHAR")
				ON_FALSE_ERROR(sscanf(next,"%d",&encoding) == 1, "Invalid Format")
				flags |=BDF_FLAG_ENCODING;
				break;
			case TOK_SWIDTH:
				ON_FALSE_ERROR(state == TOK_STARTCHAR, "Not in a STARTCHAR")
				ON_FALSE_ERROR(sscanf(next,"%d %d",&g.swidth.w,&g.swidth.h) == 2, "Invalid SWIDTH Format")
				flags |=BDF_FLAG_SWIDTH;
				break;
			case TOK_DWIDTH:
				ON_FALSE_ERROR(state == TOK_STARTCHAR, "Not in a STARTCHAR")
				ON_FALSE_ERROR(sscanf(next,"%d %d",&g.dwidth.w,&g.dwidth.h) ==2, "Invalid DWIDTH Format")
				flags |=BDF_FLAG_DWIDTH;
				break;
			case TOK_BBX:
				ON_FALSE_ERROR(state == TOK_STARTCHAR, "Not in a STARTCHAR")
				ON_FALSE_ERROR(sscanf(next,"%d %d %d %d",&g.bbx.w,&g.bbx.h,&g.bbx.x,&g.bbx.y) == 4, "Invalid BBX Format")
				flags |=BDF_FLAG_BBX;
				break;
			case TOK_BITMAP:
				ON_FALSE_ERROR(state == TOK_STARTCHAR, "Not in a STARTCHAR")
				ON_FALSE_ERROR(flags == BDF_FLAG_ALL, "STARTCHAR Header not fully set before BITMAP") 
				test = g.bbx.w * g.bbx.h,sizeof(t_byte);
				//g.bmp = (t_byte*)malloc(test);
				ON_NULL_ERROR(g.bmp, "Could not Allocate memory for Gliph")
				state = TOK_BITMAP;
				bl=0; // Current bitmap line
				break;
			case TOK_ENDFONT:
				goto exit_func;
			default:
				break;
		}
	}

exit_func:
	if(f) { fclose(f); f=NULL; }
	if(fdata) { free(fdata); fdata=NULL; }
	if(g.bmp) { free(g.bmp); g.bmp=NULL; }
	if(err_msg) 
	{
		printf("Error in reading %s\n",filename);
		if(flags & BDF_FLAG_ENCODING) 
			printf("Line(%05d) Char(%h): %s\n",linenumber,encoding,err_msg); 
		else 
			printf("Line(%05d): %s\n",linenumber,err_msg);

		exit(EXIT_FAILURE);
	}
}

*/