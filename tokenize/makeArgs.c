#include "makeArgs.h"

void clean(int argc, char **argv)
{
	int x;
	for (x = 0; x < argc; x++) {
		free(*(argv + x));
		*(argv + x) = NULL;
	}

	free(argv);
	argv = NULL;

}// end clean

void printargs(int argc, char **argv)
{
	int x;
	for(x = 0; x < argc; x++)
		printf("%s ", argv[x]);
	printf("\n");

}// end printargs

int makeargs(char *s, char *** argv)
{
	if (s == NULL) {
		return -1;
	}
	else {
		//save pointer for strtok_r
		char * save;

		//delimiters
		char delimiters[]= " \t\n";

		//make a copy of str (statically MAX, dynamically strlen)
		char * newStr = (char *) calloc(strlen(s) + 1, sizeof(char));

		strcpy(newStr, s);

		//count tokens in copy
		int tokens = 0;
		char * token = strtok_r(newStr, delimiters, &save);

		while (token != NULL) {
			tokens++;
			token =  strtok_r(NULL, delimiters, &save);
		}

		//make number of rows of char *
		*(argv) = (char **)calloc(tokens + 1, sizeof(char *));

		//pull off each token, dynamically allocate memory
		char * tempStrTok = strtok_r(s, delimiters, &save);

		int x;
		for (x = 0; x < tokens; x++) {

			char * tempStr = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));

			//strcpy into dynamically allocated memory
			strcpy(tempStr, tempStrTok);

			argv[0][x] = tempStr;

			//get next token
			tempStrTok = strtok_r(NULL, delimiters, &save);
		}

		//set last row in array to null terminator
		argv[0][tokens] = '\0';

		//if cpy of str was dynamic, you free it
		free(newStr);
		newStr = NULL;

		//return temp
		return tokens;
	}

}// end makeArgs
