
#include "bdf_parser.h"
#include <stdio.h>
#include <assert.h>
#include "missing_stl.h"



int main(int argc, char *argv[])
{
	const char* filename = "9x15.bdf";
	t_BDFInfo info;
	int error;
	error = OpenBDF(filename, &info);

	CloseBDF(&info);
	printf("Error!");
	return 0;
}