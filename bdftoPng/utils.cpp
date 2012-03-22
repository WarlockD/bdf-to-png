#include "utils.h"
#include <cstdlib>
#include <cstdarg>

#define	BADCH	(int)'?'
#define	BADARG	(int)':'
#define	EMSG	""

// shot in the dark
#ifndef _getprogname
#define _getprogname()  _nargv[0]
#endif
static std::string vformat (const char *fmt, va_list ap)
	{
		size_t size = 1024;
		char stackbuf[1024];
		std::vector<char> dynamicbuf;
		char *buf = &stackbuf[0];

		while (1) {
			int needed = vsnprintf (buf, size, fmt, ap);
        // NB. C99 (which modern Linux and OS X follow) says vsnprintf
        // failure returns the length it would have needed.  But older
        // glibc and current Windows return -1 for failure, i.e., not
        // telling us how much was needed.

        if (needed <= (int)size && needed >= 0) {
            // It fit fine so we're done.
            return std::string (buf, (size_t) needed);
        }

        // vsnprintf reported that it wanted to write more characters
        // than we allotted.  So try again using a dynamic buffer.  This
        // doesn't happen very often if we chose our initial size well.
        size = (needed > 0) ? (needed+1) : (size*2);
        dynamicbuf.resize (size);
        buf = &dynamicbuf[0];
		}
}

namespace sutil {
	
	// useful string formater
	
	std::string format (const char *fmt, ...)
	{
		va_list ap;
		va_start (ap, fmt);
		std::string buf = vformat (fmt, ap);
		va_end (ap);
		return buf;
	}





int Getopt::operator()()
{
	char *oli;				/* option letter list index */

	if (_optreset || *_place == 0) {		/* update scanning pointer */
		_optreset = false;
		_place = _nargv[_optind];
		if (_optind >= _nargc || *_place++ != '-') {
			/* Argument is absent or is not an option */
			_place = EMSG;
			return (-1);
		}
		_optopt = *_place++;
		if (_optopt == '-' && *_place == 0) {
			/* "--" => end of options */
			++_optind;
			_place = EMSG;
			return (-1);
		}
		if (_optopt == 0) {
			/* Solitary '-', treat as a '-' option
			   if the program (eg su) is looking for it. */
			_place = EMSG;
			if (strchr(_ostr, '-') == NULL)
				return (-1);
			_optopt = '-';
		}
	} else
		_optopt = *_place++;

	/* See if option letter is one the caller wanted... */
	if (_optopt == ':' || (oli = (char*)strchr(_ostr, _optopt)) == NULL) {
		if (*_place == 0)
			++_optind;
		if (_opterr && *_ostr != ':')
			(void)fprintf(stderr,
			    "%s: illegal option -- %c\n", _getprogname(),
			    _optopt);
		return (BADCH);
	}

	/* Does this option need an argument? */
	if (oli[1] != ':') {
		/* don't need argument */
		_optarg = NULL;
		if (*_place == 0)
			++_optind;
	} else {
		/* Option-argument is either the rest of this argument or the
		   entire next argument. */
		if (*_place)
			_optarg = _place;
		else if (_nargc > ++_optind)
			_optarg = _nargv[_optind];
		else {
			/* option-argument absent */
			_place = EMSG;
			if (*_ostr == ':')
				return (BADARG);
			if (_opterr)
				(void)fprintf(stderr,
				    "%s: option requires an argument -- %c\n",
				    _getprogname(), _optopt);
			return (BADCH);
		}
		_place = EMSG;
		++_optind;
	}
	return (_optopt);			/* return option letter */
}
void Getopt::reset() { _optreset = true; }

Getopt::Getopt(int nargc, char * const nargv[],const char * ostr,bool print_error) 
	:_opterr(print_error), _nargv(nargv), _nargc(nargc), _ostr(ostr), _optopt(0),_optind(0),_place(EMSG)
{
	
}

}
#ifdef _GETOPT_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int	opterr = 1,		/* if error message should be printed */
	optind = 1,		/* index into parent argv vector */
	optopt,			/* character checked for validity */
	optreset;		/* reset getopt */
char	*optarg;		/* argument associated with option */



#ifndef __getprogname
#define _getprogname() nargv[0]
#endif
/*
 * getopt --
 *	Parse argc/argv argument vector.
 */
int getopt(int nargc, char * const nargv[],const char * ostr)
{
	static char *place = EMSG;		/* option letter processing */
	char *oli;				/* option letter list index */

	if (optreset || *place == 0) {		/* update scanning pointer */
		optreset = 0;
		place = nargv[optind];
		if (optind >= nargc || *place++ != '-') {
			/* Argument is absent or is not an option */
			place = EMSG;
			return (-1);
		}
		optopt = *place++;
		if (optopt == '-' && *place == 0) {
			/* "--" => end of options */
			++optind;
			place = EMSG;
			return (-1);
		}
		if (optopt == 0) {
			/* Solitary '-', treat as a '-' option
			   if the program (eg su) is looking for it. */
			place = EMSG;
			if (strchr(ostr, '-') == NULL)
				return (-1);
			optopt = '-';
		}
	} else
		optopt = *place++;

	/* See if option letter is one the caller wanted... */
	if (optopt == ':' || (oli = (char*)strchr(ostr, optopt)) == NULL) {
		if (*place == 0)
			++optind;
		if (opterr && *ostr != ':')
			(void)fprintf(stderr,
			    "%s: illegal option -- %c\n", _getprogname(),
			    optopt);
		return (BADCH);
	}

	/* Does this option need an argument? */
	if (oli[1] != ':') {
		/* don't need argument */
		optarg = NULL;
		if (*place == 0)
			++optind;
	} else {
		/* Option-argument is either the rest of this argument or the
		   entire next argument. */
		if (*place)
			optarg = place;
		else if (nargc > ++optind)
			optarg = nargv[optind];
		else {
			/* option-argument absent */
			place = EMSG;
			if (*ostr == ':')
				return (BADARG);
			if (opterr)
				(void)fprintf(stderr,
				    "%s: option requires an argument -- %c\n",
				    _getprogname(), optopt);
			return (BADCH);
		}
		place = EMSG;
		++optind;
	}
	return (optopt);			/* return option letter */
}

#endif // _GETOPT_