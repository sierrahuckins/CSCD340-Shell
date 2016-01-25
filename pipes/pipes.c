#include "pipes.h"
#include "../tokenize/makeArgs.h"
#include "../redirect/redirect.h"
#include "../alias/alias.h"
#include "../linkedlist/linkedList.h"
#include "../linkedlist/listUtils.h"
#include "../redirect/redirect.h"

int inFD = 0;
int outFD = 1;

int containsPipe(char *s) {
	int tokens;
	
	if (s == NULL) {
		tokens = -1;
	}
	else
	{
		//save pointer for strtok_r
		char * save;

		//delimiters
		char delimiters[]= "|";

		//make a copy of str (statically MAX, dynamically strlen)
		char * newStr = (char *) calloc(strlen(s) + 1, sizeof(char));

		strcpy(newStr, s);

		//count tokens in copy
		tokens = 0;
		char * token = strtok_r(newStr, delimiters, &save);

		while (token != NULL) {
			tokens++;
			token =  strtok_r(NULL, delimiters, &save);
		}
		
		//remove one token to represent the number of pipes, not number of tokens
		tokens--;

		//if cpy of str was dynamic, you free it
		free(newStr);
		newStr = NULL;
	}
	
	//printf("The string contains %d pipe(s).\n", tokens);

	return tokens;
}

void splitForPipe(char * s, char ** leftPipe, char ** rightPipe) {
	if (s == NULL) {
		exit(-1);
	}
	else {
		//save pointer for strtok_r
		char *save;

		//delimiters
		char delimiters[] = "|";

		//copy string for tokenizing
		char *newStr = (char *) calloc(strlen(s) + 1, sizeof(char));
		strcpy(newStr, s);

		//pull off token and dynamically allocate memory
		char *tempStrTok = strtok_r(newStr, delimiters, &save);

		//create new string for left pipe
		*leftPipe = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));

		//strcpy into dynamically allocated memory
		strcpy(*leftPipe, tempStrTok);

		//pull off token and dynamically allocate memory
		tempStrTok = strtok_r(NULL, delimiters, &save);

		//create new string for left pipe
		*rightPipe = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));

		//strcpy into dynamically allocated memory
		strcpy(*rightPipe, tempStrTok);

		//free dynamically allocated memory
		free(newStr);
		newStr = NULL;
	}

}

char ** parsePrePipe(char *s, int * preCount) {
	char ** argv;
	
	if (s == NULL) {
		exit(-1);
	}
	else {
		//save pointer for strtok_r
		char * save;

		//delimiters
		char delimiters[]= "|";

		//copy string for tokenizing
		char * newStr = (char *) calloc(strlen(s) + 1, sizeof(char));

		strcpy(newStr, s);

		//pull off token and dynamically allocate memory
		char * tempStrTok = strtok_r(newStr, delimiters, &save);

		char * tempStr = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));

		//strcpy into dynamically allocated memory
		strcpy(tempStr, tempStrTok);

		//check if contains a redirect
		if (strstr(tempStr, "<") != NULL) {
			*preCount = redirectIn(tempStr, &argv, &inFD);
		}
		else if (strstr(tempStr, ">") != NULL) {
			*preCount = redirectOut(tempStr, &argv, &outFD);
		}
			//else call makeargs with only poststring
		else {
			*preCount = makeargs(tempStr, &argv);
		}

		//print result to screen
		printf("The prePipe string is: ");
		printargs(*preCount, argv);

		//free dynamically allocated memory
		free(newStr);
		newStr = NULL;
		free(tempStr);
		newStr = NULL;

		return argv;
	}
}

char ** parsePostPipe(char *s, int * postCount){
	char ** argv;
	
	if (s == NULL) {
		exit(-1);
	}
	else {
		//save pointer for strtok_r
		char * save = NULL;

		//delimiters
		char delimiters[]= "|";

		//copy string for tokenizing
		char * newStr = (char *) calloc(strlen(s) + 1, sizeof(char));

		strcpy(newStr, s);

		//pull off token, dynamically allocate memory
		char * tempStrTok = strtok_r(newStr, delimiters, &save);

		//throw away first token and pull off second
		tempStrTok = strtok_r(NULL, delimiters, &save);

		char * tempStr = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));

		//strcpy into dynamically allocated memory
		strcpy(tempStr, tempStrTok);

		//check if contains a redirect
		if (strstr(tempStr, "<") != NULL) {
			*postCount = redirectIn(tempStr, &argv, &inFD);
		}
		else if (strstr(tempStr, ">") != NULL) {
			*postCount = redirectOut(tempStr, &argv, &outFD);
		}
		//else call makeargs with only poststring
		else {
			*postCount = makeargs(tempStr, &argv);
		}

		//print result to screen
		printf("The postPipe string is: ");
		printargs(*postCount, argv);

		//free dynamically allocated memory
		free(newStr);
		newStr = NULL;
		free(tempStr);
		newStr = NULL;

		return argv;
	}
}

void pipeIt(char * s, LinkedList * aliasList){
	char ** argv;
	int argc;
	int preCount = 0, postCount = 0;
	char ** prePipe,  ** postPipe;
	alias * checkAliasArgs = NULL;

	//////////////////////////////////
	pid_t pid;
	int fd[2], res, status;

	pid = fork();
	
	if(pid != 0) {
		waitpid(pid, &status, 0);
	}
	
	else {
		res = pipe(fd);

		if(res < 0)
		{
			printf("Pipe Failure\n");
			exit(-1);
		}// end if

		pid = fork();

		if(pid != 0)
		{
			//split off post pipe
			postPipe = parsePostPipe(s, &postCount);
			argv = postPipe;

			//check if postPipe is an alias and update if necessary
			if (postCount == 1) {
				checkAliasArgs = (alias *)buildAliasType_Args(argv);
				Node * postPipeNode = buildNode_Type(checkAliasArgs);

				int foundAlias = 0;
				foundAlias = findInList(aliasList, postPipeNode, compareAlias);

				if (foundAlias == 1) {
					checkAliasArgs = (alias *) postPipeNode->data;

					//clean our makeargs
					//clean(argc, argv);

					//and replace with new
					argc = makeargs(checkAliasArgs->argv[1], &argv);
				}
				free(postPipeNode->data);
				postPipeNode->data = NULL;

				free(postPipeNode);
				postPipeNode = NULL;
			}

			//close the write end of the parent's pipe
			close(fd[1]);

			//alias the read end of the pipe to stdin
			close(0);
			dup(fd[0]);
		
			close(fd[0]);
			int result = execvp(postPipe[0], postPipe);
			
			//deal with bad result from exec
			if (result == -1) {
				exit(-1);
			}
	
		}// end if AKA parent
		else
		{
			//split off prepipe
			prePipe = parsePrePipe(s, &preCount);
			argv = prePipe;

			//check if prePipe is an alias and update if necessary
			if (preCount == 1) {
				checkAliasArgs = (alias *)buildAliasType_Args(argv);
				Node * prePipeNode = buildNode_Type(checkAliasArgs);

				int foundAlias = 0;
				foundAlias = findInList(aliasList, prePipeNode, compareAlias);

				if (foundAlias == 1) {
					checkAliasArgs = (alias *) prePipeNode->data;

					//clean our makeargs
					//clean(argc, argv);

					//and replace with new
					argc = makeargs(checkAliasArgs->argv[1], &argv);
				}
				free(prePipeNode->data);
				prePipeNode->data = NULL;

				free(prePipeNode);
				prePipeNode = NULL;
			}

			close(fd[0]);
			close(1);
			dup(fd[1]);
			close(fd[1]);
			int result = execvp(prePipe[0], prePipe);

			//deal with bad result from exec
			if (result == -1) {
				exit(-1);
			}
		}// end else AKA child
	}

	clean(preCount, prePipe);
	clean(postCount, postPipe);
}
