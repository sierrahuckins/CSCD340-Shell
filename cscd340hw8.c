#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

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
	fp = fopen(".msshrc", "r");

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
	fp = fopen(".mssh_history", "r");

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
		fp = fopen(".mssh_history", "w");
	}

	//close .mssh_history
	fclose(fp);

	//printList(historyList, printHistoryType, 0);

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
			//printList(historyList, printHistoryType, (historyList->size - histcount));
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
			checkForAlias(&leftPipe, aliasList);
			checkForAlias(&rightPipe, aliasList);

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
			char *command;
			char *inRedirect = NULL;
			char *outRedirect = NULL;

			//check for redirections
			checkForRedirection(s, &command, &inRedirect, &outRedirect);

			//check for alias
			checkForAlias(&command, aliasList);

			//makeargs for command
			argc = makeargs(command, &argv);

			/****************
			 * FORK A CHILD FOR EXECUTION
			 ****************/
			pid_t pid;
			int res, status;
			pid = fork();

			//parent will wait so it can return to original program
			if (pid == 0) {
				/****************
				 * SETUP REDIRECTION
				 ****************/
				if (inRedirect != NULL)
					redirectIn(inRedirect);
				if (outRedirect != NULL)
					redirectOut(outRedirect);

				/****************
				 * HANDLE COMMANDS WITH NEW PROGRAMS
				 ****************/

				//normal execution
				/*if (pathSet == TRUE) {
					res = execl(path, argv[0], argv);
				}
				else {*/
					res = execvp(argv[0], argv);
				//}

				//deal with bad result from exec
				if (res == -1)
					exit(-1);
			}
			else {
				//will wait for child to return
				waitpid(pid, &status, 0);

				/****************
				 * HANDLE INTERNAL COMMANDS
				 ****************/

				//special case: alias
				if (strcmp(argv[0], "alias") == 0) {
					//make new alias style args
					argc = makealiasargs(s, &argv);

					//add to alias list
					addLast(aliasList, buildNode_Type(buildAliasType_Args(argv)));

					printList(aliasList, printAliasType, 0);
				}
				//special case: unalias
				if (strcmp(argv[0], "unalias") == 0) {
					checkForAliasToRemove(argv[1], aliasList);
					printList(aliasList, printAliasType, 0);
				}
				//special case:cd
				if (strcmp(argv[0], "cd") == 0) {
					chdir(argv[1]);

				}
				//special case: history
				if (strcmp(argv[0], "history") == 0) {
					printList(historyList, printHistoryType, (historyList->size - histcount));
				}
				//special case: exclamation points
				if (*(argv[0]) == '!') {
					if (*(argv[0] + 1) == '!') {
						//get last history
						Node * lastHistory= retrieveLast(historyList);
						history * lastCommand = lastHistory->data;

						if (pathSet == TRUE) {
							res = execl(path, lastCommand->argv[0], lastCommand->argv);
						}
						else {
							res =execvp(lastCommand->argv[0], lastCommand->argv);
						}
						exit(0);
					}
					else {
						//still need to figure this one out!!
					}
				}
				//special case: setting path
				else if (strstr(argv[0], "PATH=$PATH") == argv[0]) {
					//PATH=$PATH\:/dir/path
					//save pointer for strtok_r
					char * save;

					if (pathSet == TRUE) {
						char * newpath = (char *) calloc(strlen(argv[0]) + 1 - 6 + strlen(path) + 1, sizeof(char));

						//we need to remove the front part
						char * truncatedStrTok = strtok_r(argv[0], ":", &save);

						//copy to path variables
						strcpy(newpath, path);
						strcat(newpath, ":");
						strcat(newpath, truncatedStrTok);

						//free old path
						free(path);
						path = NULL;

						//calloc new memory and copy into it
						path = (char *) calloc(strlen(newpath), sizeof(char));
						strcpy(path, newpath);

						//free newpath memory
						free(newpath);
						newpath = NULL;
					}
					else {
						//we need to remove the front part
						char * truncatedStrTok = strtok_r(argv[0], ":", &save);

						//calloc new memory and copy into it
						path = (char *) calloc(strlen(argv[0]) + 1 - 6 + strlen(path) + 1, sizeof(char));
						strcpy(path, save);

						//set path flag
						pathSet = TRUE;
					}
				}
			}

			//free my command string
			free(command);
			command = NULL;

			//free make args
			clean(argc, argv);
			argv = NULL;

			//free redirect paths if necessary
			if (inRedirect != NULL) {
				free(inRedirect);
				inRedirect = NULL;
			}
			if (outRedirect != NULL){
				free(outRedirect);
				outRedirect = NULL;
			}
		}

		printf("command?: ");
	  	fgets(s, MAX, stdin);
		strip(s);

  	}// end while

	//write out to .msshrc_history
	fp = fopen(".mssh_history", "w");

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
				fputs(listing->argv[z], fp);
				fputs(" ", fp);
			}

			fputs("\n", fp);
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

