/*
* Win32 unistd.h
*
* Replaces Unix unistd.h
* Originally by Ulrik Petersen, modified by Charlie Hull of Lemur Consulting Ltd.
*/
#ifdef __WIN32__
#include <direct.h>
#include <io.h>
#define mode_t int
#define S_ISREG(m) (((m)&_S_IFMT) == _S_IFREG)
#define S_ISDIR(m) (((m)&_S_IFMT) == _S_IFDIR)
#define mkdir _mkdir
#define S_IRUSR  (_S_IREAD)
#define S_IWUSR  (_S_IWRITE)
#define ssize_t  SSIZE_T
/*
--This removed for Xapian 0.9.9 - it caused obscure link problems with Flint. 
#define open _open
*/
#undef min
#define min _cpp_min
#undef max
#define max _cpp_max
#endif