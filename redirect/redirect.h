#ifndef REDIRECT_H
#define REDIRECT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "../tokenize/makeArgs.h"
#include <fcntl.h>
#include <sys/stat.h>

int redirectIn(char *s, char *** argv);
int redirectOut(char *s, char *** argv);

#endif 

