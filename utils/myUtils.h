#ifndef MYUTILS_H
#define MYUTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../linkedlist/linkedList.h"
#include "../redirect/redirect.h"

#define MAX 100

void strip(char *array);
void checkForAlias(char * s, char ** command, LinkedList * aliasList);
void checkForRedirection(char * s, char ** command, char ** redirectInPath, char ** redirectOutPath);
void checkForAliasToRemove(char * s, LinkedList * aliasList);

#endif
