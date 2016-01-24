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

int makeargs(char *s, char *** argv) {
	if (s == NULL) {
		return -1;
	}
	else {
//save pointer for strtok_r
		char *save1, *save2;

//delimiters
		char delimiters[] = " \t\n";

//make a copy of str (statically MAX, dynamically strlen)
		char newStr1[MAX];
		strcpy(newStr1, s);

		//char *newStr2 = (char *) calloc(strlen(s) + 1, sizeof(char));
		char newStr2[MAX];
		strcpy(newStr2, s);

//count tokens in copy
		int tokens = 0;
		char *token = strtok_r(newStr1, delimiters, &save1);

		while (token != NULL) {
			tokens++;
			token = strtok_r(NULL, delimiters, &save1);
		}

//make number of rows of char *
		*(argv) = (char **) calloc(tokens + 1, sizeof(char *));

//pull off each token, dynamically allocate memory
		char *tempStrTok = strtok_r(newStr2, delimiters, &save2);

		int x = 0;
		while (tempStrTok != NULL) {
			(*argv)[x] = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));

//strcpy into dynamically allocated memory
			strcpy((*argv)[x], tempStrTok);
			x++;

//get next token
			tempStrTok = strtok_r(NULL, delimiters, &save2);
		}

//set last row in array to null terminator
		(*argv)[tokens] = '\0';

//return temp
		return tokens;
	}
}
