
#include "bdf_parser.h"
#include <stdio.h>
#include <assert.h>
#include "dirent.h"



int main2(int argc, char *argv[])
{
	DIR *dir = NULL;
	struct dirent* ent=NULL;
	const char* filename = "9x15.bdf";
	t_sBDFStream stream;
	t_BDFInfo info;
	int error;
	dir = opendir(".");
	for(ent = readdir(dir);ent != NULL; ent = readdir(dir)) 
		printf(".%s\n",ent->d_name);
	error = OpenBDF(filename, &stream);


	if(error == BDF_NO_ERROR) {
		error = ParseBDF(&stream,&info);	
	}
	printf("Error!");
	return 0;
}