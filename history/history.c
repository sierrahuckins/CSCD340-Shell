#include "history.h"

static int count = 1;

void incrementHistoryCount() {
	count++;
}

/**
 * @brief The Make Args function from the last lab
 *
 * Copy and Paste MakeArgs from the last lab
 *
 * @param s - The String to be tokenized
 * @param argv - The array to put the tokens into
 *
 * @warning - If there are no tokens or the String cant be parsed - exit(-99) 
 */
int makehistoryargs(char *s, char *** argv) {
	if (s == NULL) {
		return -1;
	}
	else {
		//save pointer for strtok_r
		char * save;

		//delimiters
		char delimiters[]= " \t\n";

//make a copy of str (statically MAX, dynamically strlen)
		char newStr1[MAX];
		strcpy(newStr1, s);

		//char *newStr2 = (char *) calloc(strlen(s) + 1, sizeof(char));
		char newStr2[MAX];
		strcpy(newStr2, s);

		//count tokens in copy
		int tokens = 0;
		char * token = strtok_r(newStr1, delimiters, &save);

		while (token != NULL) {
			tokens++;
			token =  strtok_r(NULL, delimiters, &save);
		}

		//make number of rows of char *
		*(argv) = (char **)calloc(tokens + 1, sizeof(char *));

		//copy in s again
		//strcpy(newStr, s);

		//pull off each token, dynamically allocate memory
		char * tempStrTok = strtok_r(newStr2, delimiters, &save);

		int x;
		for (x = 0; x < tokens; x++) {

			(*argv)[x] = (char *)calloc(strlen(tempStrTok) + 1, sizeof(char));

			//strcpy into dynamically allocated memory
			strcpy((*argv)[x], tempStrTok);

			//get next token
			tempStrTok = strtok_r(NULL, delimiters, &save);
		}

		//set last row in array to null terminator
		argv[0][tokens] = '\0';

		//return temp
		return tokens;
	}
}

/**
 * @brief The builds the history data structure
 *
 * Builds the history structure and returns it as a void pointer
 *
 * @param argc - The int for the number of arguments
 * @param argv - The array to put the tokens into
 * @return void * - Representing the data structure history
 *
 * @warning - If there are no tokens or the String is null - exit(-99) 
 */
void * buildHistoryType_Args(int argc, char **argv) {
	history * data = (history *)calloc(1,sizeof(history));
	
	data->argc = argc;
	data->argv = argv;

	data->number = count;
	
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
void printHistoryType(void * passedIn) {
	history * data = (history *)passedIn;

	printf("%d ", data->number);

	int x;
	for(x = 0; x < data->argc; x++)
		printf("%s ", data->argv[x]);

	
	printf("\n");
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
int compareHistory(const void * p1, const void * p2) {
	history * h1 = (history *)p1;
	history * h2 = (history *)p2;

	if (h1->argc > h2->argc)
		return -1;
	else if (h1->argc < h2->argc)
		return 1;
	else {
		int x;
		for (x = 0; x < h1->argc; x++) {
			if (strcmp(h1->argv[x], h2->argv[x]) != 0)
				return strcmp(h1->argv[x], h2->argv[x]);
		}
		return 0;
	}

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
void cleanTypeHistory(void * passedIn) {
	history * data = (history *)passedIn;

	int x;
	for (x = 0; x <= data->argc; x++) {
		free(data->argv[x]);
		data->argv[x] = NULL;
	}

	free(data->argv);
	data->argv = NULL;

	free(data);
	data = NULL;
}

