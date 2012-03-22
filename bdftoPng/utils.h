#ifndef _UTILS_H_
#define _UTILS_H_

#if defined(_WIN32) || defined(WIN32)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include <string>
#include <vector>

namespace sutil {
	std::string format (const char *fmt, ...);
	char* strok_line(char**next); 
	char* strtok_r(char *s, const char *delim, char **last);
	int strcasecmp(char * s1,char * s2);
	class Getopt {
	private:
		const char * _ostr;
		char * const * _nargv;
		int _nargc;
		char* _place;
		bool _optreset;
		bool _opterr;		/* if error message should be printed */
		int _optind;		/* index into parent argv vector */
	    int _optopt;		/* character checked for validity */
		char *_optarg;
       // std::string _optarg;		/* argument associated with option */
	public:
		inline const char* optarg() const { return _optarg; }
		inline int optind() const { return _optind; }
		inline int optopt() const { return _optind; }
		void reset();
		Getopt(int nargc, char * const nargv[],const char * ostr,bool print_error=true);
		int operator()();
	};

}

#endif