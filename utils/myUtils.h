#ifndef MYUTILS_H
#define MYUTILS_H

#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../linkedlist/linkedList.h"
#include "../redirect/redirect.h"

void strip(char *array);
void checkForAlias(char ** command, LinkedList * aliasList);
void checkForRedirection(char * s, char ** command, char ** redirectInPath, char ** redirectOutPath);
void checkForAliasToRemove(char * s, LinkedList * aliasList);

#endif
