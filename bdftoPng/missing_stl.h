#ifndef _MISSING_STL_H_
#define _MISSING_STL_H_

#define _GETOPT_  // define if you need getopt
#define _DIRENT_ // Define for directory functions
#define _STRCASECMP_ // string comare regardless of case
#define _STROK_R_ // strok_r impltation and one using just line

#if defined(_WIN32) || defined(WIN32)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _STROK_R_
	char* strok_line(char**next); 
	char* strtok_r(char *s, const char *delim, char **last);
#endif

#ifdef _STRCASECMP_
	int strcasecmp(char * s1,char * s2);
#endif

#ifdef _GETOPT_
	extern char* optarg;
	extern int opterr,optind,optopt,optreset;
	int getopt(int nargc, char * const nargv[],const char * ostr);
#endif //_STRCASECMP_

#ifdef _DIRENT_
	typedef struct DIR DIR;
	struct dirent { char *d_name; };
	DIR           *opendir(const char *);
	int           closedir(DIR *);
	struct dirent *readdir(DIR *);
	void          rewinddir(DIR *);
#endif // _DIRENT_

#ifdef __cplusplus
}
#endif

#endif