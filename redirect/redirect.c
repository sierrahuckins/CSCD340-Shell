#include "redirect.h"

void redirectIn(char * s, char ** command){
	if (s == NULL) {
		exit(-1);
	}
	else {
		//save pointer for strtok_r
		char * save = NULL;

		//delimiters
		char delimiters[]= "<";

		//copy string for tokenizing
		char * newStr = (char *) calloc(strlen(s) + 1, sizeof(char));

		strcpy(newStr, s);

		//pull off token, dynamically allocate memory
		char * tempStrTok = strtok_r(newStr, delimiters, &save);

		//dynamically allocate new memory
		*command = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
		strcpy(*command, tempStrTok);

		//get right side of token
		tempStrTok = strtok_r(NULL, delimiters, &save);

		char * newIn = (char *)calloc(strlen(tempStrTok) + 1, sizeof(char));
		strcpy(newIn, tempStrTok);

		//redirect stdin using newIn
		int fd = open(newIn, O_CREAT|O_APPEND|O_RDONLY, 0777);
		close(0);
		dup(fd);

		//free dynamically allocated memory
		free(newStr);
		newStr = NULL;
		free(newIn);
		newIn = NULL;
	}
}

void redirectOut(char * s, char ** command) {
	if (s == NULL) {
		exit(-1);
	}
	else {
		//save pointer for strtok_r
		char * save = NULL;

		//delimiters
		char delimiters[]= ">";

		//copy string for tokenizing
		char * newStr = (char *) calloc(strlen(s) + 1, sizeof(char));

		strcpy(newStr, s);

		//pull off token, dynamically allocate memory
		char * tempStrTok = strtok_r(newStr, delimiters, &save);

		//dynamically allocate new memory
		*command = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
		strcpy(*command, tempStrTok);

		//get right side of token
		tempStrTok = strtok_r(NULL, delimiters, &save);

		char * newOut = (char *)calloc(strlen(tempStrTok) + 1, sizeof(char));
		strcpy(newOut, tempStrTok);

		//redirect stdin using newIn
		int fd = open(newOut, O_CREAT|O_APPEND|O_RDONLY, 0777);
		close(1);
		dup(fd);

		//free dynamically allocated memory
		free(newStr);
		newStr = NULL;
		free(newOut);
		newOut = NULL;
	}
}
