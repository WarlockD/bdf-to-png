
#ifdef _STROK_R_
#include <string.h>

// Its funny.  I started out wanting to use gotos on this but ended up
// using a bunch of for loops?  I wonder witch is more readable?
// The freebsd/netbsd version of strok_r is just funky though
// Some reason this looks sexy too.  merrow
char* strok_line(char**next) {
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

char* strtok_r(char *s, const char *delim, char **last)
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
#endif //_STROK_R_