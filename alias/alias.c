#include "alias.h"

int makealiasargs(char *s, char *** argv) {
	if (s == NULL) {
		return -1;
	}
	else {
		//save pointer for strtok_r
		char * save;

		//delimiters
		char delimiters[]= "=\"";

		//make a copy of str (statically MAX, dynamically strlen)
		char * newStr = (char *) calloc(strlen(s) + 1, sizeof(char));

		strcpy(newStr, s);

		//count tokens in copy
		int tokens = 2;

		*(argv) = (char **)calloc(tokens + 1, sizeof(char *));

		//pull off each token, dynamically allocate memory
		char * tempStrTok = strtok_r(s, delimiters, &save);

		int x;
		for (x = 0; x < tokens; x++) {

			//if this is an alias we need to remove the front part
			if (x == 0 && strcmp(tempStrTok, "PATH") != 0) {
				(*argv)[x] = (char *)calloc(strlen(tempStrTok) + 1 - 6, sizeof(char));
				strcpy((*argv)[x], &tempStrTok[6]);
			}
			else {
				(*argv)[x] = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
				strcpy((*argv)[x], tempStrTok);
			}

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
}

/**
 * @brief The builds the alias data structure
 *
 * Builds the history structure and returns it as a void pointer
 *
 * @param argc - The int for the number of arguments
 * @param argv - The array to put the tokens into
 * @return void * - Representing the data structure history
 *
 * @warning - If there are no tokens or the String is null - exit(-99) 
 */
void * buildAliasType_Args(char **argv) {
	alias * data = (alias *)calloc(1,sizeof(alias));
	
	data->argv = argv;
	
	return data;
}

/**
 * @brief Prints the specific types contents.
 *
 * The printType function is called via function pointer
 * Its focus is to print the specific type formatted for
 * the type
 *
 * @param passedIn - The void * passed in representing the specific data
 *
 * @warning - The passed in void * passedIn is checked - exit(-99) if NULL
 */
void printAliasType(void * passedIn) {
	alias * data = (alias *)passedIn;

	printf("%s=\"%s\"\n", data->argv[0], data->argv[1]);
}

/**
 * @brief Compares two objects of the specific data type.
 *
 * Compares two object of the specific data type and emulates the
 * behavior of the Java compareTo method indicating a sense of order.
 * The data checked is determined by the specifications provided by
 * the user.
 *
 * @note The passed in items will need to be cast to the appropriate
 * specific data type.
 *
 * @param p1 - The void * representing an object of the specific data type
 * @param p2 - The void * representing an object of the specific data type
 * @return int - Representing order < 0 indicates p1 is less than p2,
 * > 0 indicates p1 is greater than p2 and == 0 indicates p1 == p2 for contents
 *
 * @warning - The passed in void * p1 is checked - exit(-99) if NULL
 * @warning - The passed in void * p2 is checked - exit(-99) if NULL
 */
int compareAlias(const void * p1, const void * p2) {
	alias * one = (alias *)p1;
	alias * two = (alias *)p2;

	return strcmp(one->argv[0], two->argv[0]);
}


/**
 * @brief Cleans up all dynamically allocated memory for the specific data type
 *
 * Cleans up and frees all the dynamically allocated memory for the
 * specific data type.  Each pointer in the specific data type is set
 * to NULL after it has been freed.
 *
 * @param passedIn - The void * passed in representing the specific data
 *
 * @warning - The passed in void * passedIn is checked - exit(-99) if NULL
 */
void cleanTypeAlias(void * passedIn) {
	alias * data = (alias *)passedIn;

	int x;
	for (x = 0; x <= 2; x++) {
		free(data->argv[x]);
		data->argv[x] = NULL;
	}

	free(data->argv);
	data->argv = NULL;

	free(data);
	data = NULL;
}

