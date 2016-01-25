#include "redirect.h"

int redirectIn(char *s, char *** argv){
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

		char * tempStr = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));

		//strcpy into dynamically allocated memory
		strcpy(tempStr, tempStrTok);

		//get right side of token
		tempStrTok = strtok_r(NULL, delimiters, &save);

		char * newIn = (char *)calloc(strlen(tempStrTok) + 1, sizeof(char));

		strcpy(newIn, tempStrTok);

		//redirect stdin using newIn
		int fd = open(newIn, O_CREAT|O_APPEND|O_RDONLY, 0777);
		close(0);
		dup(fd);

		//check if contains a redirect
		int argc = makeargs(tempStr, argv);
		
		//free dynamically allocated memory
		free(newStr);
		newStr = NULL;
		free(tempStr);
		newStr = NULL;
		free(newIn);
		newIn = NULL;

		return argc;
	}
}

int redirectOut(char *s, char *** argv){
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

		char * tempStr = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));

		//strcpy into dynamically allocated memory
		strcpy(tempStr, tempStrTok);

		//get right side of token
		tempStrTok = strtok_r(NULL, delimiters, &save);

		char * newOut = (char *)calloc(strlen(tempStrTok) + 1, sizeof(char));

		strcpy(newOut, tempStrTok);

		//redirect stdout using newIn
		int fd = open(newOut, O_CREAT|O_APPEND|O_WRONLY, 0777);
		close(1);
		dup(fd);

		//check if contains a redirect
		int argc = makeargs(tempStr, argv);
		
		//free dynamically allocated memory
		free(newStr);
		newStr = NULL;
		free(tempStr);
		newStr = NULL;
		free(newOut);
		newOut = NULL;

		return argc;
	}
}
