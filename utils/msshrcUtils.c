#include "msshrcUtils.h"
#include "myUtils.h"

int getHistCount(FILE * fp, char * s) {
	//get histcount line from file
	fgets(s, MAX, fp);
	strip(s);
	char delimiters[]= "=";

	//pull off token, dynamically allocate memory
	char * tempStrTok = strtok(s, delimiters);

	//convert to num and save
	int num = atoi(s);

	return num;
}

int getHistFileCount(FILE * fp, char * s) {
	//save pointer for strtok_r
	char * save;

	//get histfilecount line from file
	fgets(s, MAX, fp);
	strip(s);
	char delimiters[]= "=";

	//pull off token, dynamically allocate memory
	char * tempStrTok = strtok_r(s, delimiters, &save);

	//convert to num and save
	int num = atoi(save);

	return num;
}
