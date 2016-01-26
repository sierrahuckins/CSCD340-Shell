#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "./utils/myUtils.h"
#include "./utils/msshrcUtils.h"
#include "./history/history.h"
#include "./linkedlist/listUtils.h"
#include "./linkedlist/linkedList.h"
#include "./alias/alias.h"
#include "./pipes/pipes.h"
#include "./tokenize/makeArgs.h"
#include "./redirect/redirect.h"

#define MAX 100
#define TRUE 1
#define FALSE 0

int main()
{
	int argc, histcount, histfilecount;
	char **argv = NULL, s[MAX], temp[MAX];
	char * path = NULL;
	int pathSet = FALSE;
	LinkedList * aliasList = linkedList();
	LinkedList * historyList = linkedList();
	char ** prePipe = NULL, ** postPipe = NULL;
	FILE * fp = NULL;
	FILE * tempFile = NULL;
	alias * checkAliasArgs = NULL;

	//check if .msshrc exists and setup variables if it does
	fp = fopen("/home/shuckins/.msshrc", "r");

	if (fp != NULL) {
		//get histcount from file
		histcount = getHistFileCount(fp, s);

		//get histfilecount from file
		histfilecount = getHistFileCount(fp, s);

		//get aliases
		fgets(s, MAX, fp);

		//if the line that was just grabbed was empty, move forward a line
		while (strcmp(s, "\n") == 0 && !feof(fp))
			fgets(s, MAX, fp);

		strip(s);

		if (!feof(fp)) {

			argc = makealiasargs(s, &argv);

			while (strcmp(argv[0], "PATH") != 0 && !feof(fp)) {
				addLast(aliasList, buildNode_Type(buildAliasType_Args(argv)));

				fgets(s, MAX, fp);

				//if the line that was just grabbed was empty, move forward a line
				while (strcmp(s, "\n") == 0 && !feof(fp))
					fgets(s, MAX, fp);

				if (!feof(fp)) {
					strip(s);
					argc = makealiasargs(s, &argv);
				}
			}

			//set path variable
			if (!feof(fp)) {
				//save pointer for strtok_r
				char * save;

				path = (char *) calloc(strlen(argv[1]) + 1 - 6, sizeof(char));

				//we need to remove the front part
				char * truncatedStrTok = strtok_r(argv[1], ":", &save);

				//copy to path variables
				strcpy(path, save);

				//set path flag
				pathSet = TRUE;

				//clean up the makealiasargs call
				cleanTypeAlias(buildAliasType_Args(argv));
			}
		}

		//close .msshrc
		fclose(fp);
	}
	else {
		//printf("Error opening file: %s.\n", strerror(errno));
		histcount = 100;
		histfilecount = 1000;
	}

	printList(aliasList, printAliasType, 0);

	strcpy(s, "");

	//check if .mssh_history exists
	fp = fopen("/home/shuckins/.mssh_history", "r");

	if (fp != NULL) {
		//find out how many lines are in history
		fgets(s, MAX, fp);

		while (!feof(fp)) {
			argc = makehistoryargs(s, &argv);
			addLast(historyList, buildNode_Type(buildHistoryType_Args(argc, argv)));
			incrementHistoryCount();
			fgets(s, MAX, fp);
		}
	}
	//else if it doesn't exist, create it
	else {
		fp = fopen("/home/shuckins/.mssh_history", "w");
	}

	//close .mssh_history
	fclose(fp);

	printList(historyList, printHistoryType, 0);

	//request command from user
  	printf("command?: ");
  	fgets(s, MAX, stdin);
  	strip(s);

	//handle command if it wasn't exit
  	while(strcmp(s, "exit") != 0) {
		/****************
		 * HANDLE HISTORY
		 ****************/
		argc = makehistoryargs(s, &argv);
		history * currentArgs = (history *)buildHistoryType_Args(argc, argv);
		Node * lastHistory= retrieveLast(historyList);

		//either add to history or clean up the args we've made if it already matches last history entry
		if(historyList->size == 0 || compareHistory(currentArgs, lastHistory->data) != 0) {
			addLast(historyList, buildNode_Type(currentArgs));
			incrementHistoryCount();
			printList(historyList, printHistoryType, (historyList->size - histcount));
		}
		else {
			//clean up the makehistoryargs call
			cleanTypeHistory(currentArgs);
		}

		/****************
		 * HANDLE PIPE
		 ****************/
		int pipes = containsPipe(s);

		if (pipes > 0) {
			//split into left and right strings
			char * leftPipe, * rightPipe;
			splitForPipe(s, &leftPipe, &rightPipe);

			//check for aliases
			checkForAlias(leftPipe, &leftPipe, aliasList);
			checkForAlias(rightPipe, &rightPipe, aliasList);

			/****************
			 * PIPEIT WILL GO HERE
			 ****************/

			//free my pipe strings
			free(leftPipe);
			leftPipe = NULL;
			free(rightPipe);
			rightPipe = NULL;
		}
		/****************
		 * HANDLE SINGLE COMMAND
		 ****************/
		else {
			char * command;
			char * inRedirect = NULL;
			char * outRedirect = NULL;

			//check for redirections
			checkForRedirection(s, &command, &inRedirect, &outRedirect);

			//check for alias
			checkForAlias(s, &command, aliasList);

			//makeargs for command
			argc = makeargs(command, &argv);

			/****************
			 * FORK A CHILD FOR EXECUTION
			 ****************/
			pid_t pid;
			int res, status;
			pid = fork();

			//parent will wait so it can return to original program
			if(pid != 0) {
				waitpid(pid, &status, 0);
			}
			//child will execute command
			else {
				/****************
				 * SETUP REDIRECTION
				 ****************/
				if (inRedirect != NULL)
					redirectIn(inRedirect);
				if (outRedirect != NULL)
					redirectOut(outRedirect);

				/****************
				 * HANDLE COMMAND
				 ****************/
				//special case: alias
				if (strcmp(argv[0], "alias") == 0) {
					addLast(aliasList, buildNode_Type(buildAliasType_Args(argv)));
				}
				//special case: unalias
				else if (strcmp(argv[0], "unalias") == 0) {
					checkForAliasToRemove(argv[1], aliasList);
				}
				//special case:cd
				else if (strcmp(argv[0], "cd") == 0) {
					//still need to figure this one out!!
				}
				//special case: history
				else if (strcmp(argv[0], "history") == 0) {
					printList(historyList, printHistoryType, (historyList->size - histcount));
				}
				//special case: exclamation points
				else if (*(argv[0] + 1) == '!') {
					if (*(argv[0] + 2) == '!') {
						//still need to figure this one out!!
					}
					else {
						//still need to figure this one out!!
					}
				}
				//special case: setting path
				else if (strcmp(argv[0], "path") == 0) {
					//think this might be PATH???
					//figure this one out!
				}
				//normal execution
				else {
					if (pathSet == TRUE) {
						execlp(path, argv[0], argv);
					}
					else {
						execvp(argv[0], argv);
					}
				}
			}

			//free my command string
			free(command);
			command = NULL;
		}

		printf("command?: ");
	  	fgets(s, MAX, stdin);
		strip(s);

  	}// end while

	//write out to .msshrc_history
	fp = fopen("/home/shuckins/.mssh_history", "r");

	if (fp != NULL) {
		int skip = historyList->size - histfilecount;
		if (skip < 0)
			skip = 0;
		int x;
		for (x = 0; x < skip; x++) {
			removeFirst(historyList, cleanTypeHistory);
		}

		//read historylist into templist
		while (historyList->size > 0) {
			Node * curr = retrieveFirst(historyList);
			history * listing = (history *)curr->data;

			int z;
			for (z = 0; z < listing->argc; z++) {
				fputs(listing->argv[z], tempFile);
				fputs(" ", tempFile);
			}

			fputs("\n", tempFile);
			removeFirst(historyList, cleanTypeHistory);
		}

		//close files
		fclose(fp);
	}
	//else report error
	else {
		printf("There was an error opening .msshrc_history. No history written.");
	}

	if (pathSet == TRUE) {
		free(path);
		path = NULL;
	}

	clearList(historyList, cleanTypeHistory);
   	free(historyList);
   	historyList = NULL;

	clearList(aliasList, cleanTypeAlias);
   	free(aliasList);
   	aliasList = NULL;

   	printf("Program Ended\n");

  	return 0;

}// end main

