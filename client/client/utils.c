#include "utils.h"
#include<stdlib.h>
#include<string.h>

//*******************************************************
// ConcatString - concat 3 stings
//*******************************************************
char* ConcatString(	char* source_a, char* source_b,char* source_c)
{
	int total_size = strlen(source_a)+strlen(source_b)+strlen(source_c);
	char* string = (char*) malloc(total_size* sizeof(char));
	if (string == NULL)
	{
		exit(1);
	}
	strcpy(string,source_a);
	strcat(string,source_b);
	strcat(string,source_c);
	return string;
}