#ifndef HISTORY_H
#define HISTORY_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX 100

struct hist
{
    int argc;
    char ** argv;
    int number;
};

typedef struct hist history;

void incrementHistoryCount();

int makehistoryargs(char *s, char *** argv);

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
void * buildHistoryType_Args(int argc, char **argv);

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
void printHistoryType(void * passedIn);

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
int compareHistory(const void * p1, const void * p2);

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
void cleanTypeHistory(void * passedIn);

#endif
