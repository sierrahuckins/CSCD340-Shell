#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/limits.h>


#include "./utils/myUtils.h"
#include "./utils/msshrcUtils.h"
#include "./history/history.h"
#include "./linkedlist/listUtils.h"
#include "./linkedlist/linkedList.h"
#include "./alias/alias.h"
#include "./pipes/pipes.h"
#include "./tokenize/makeArgs.h"
#include "./redirect/redirect.h"

#define MAX 500
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
	char * startingDir;
	char tempPath[PATH_MAX];

	//check if .msshrc exists and setup variables if it does
	fp = fopen(".msshrc", "r");
	startingDir=getcwd(tempPath, PATH_MAX);

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

			/****************
			 * SET ALIASES
			 ****************/
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

			/****************
			 * SET PATH VARIABLE
			 ****************/
			if (!feof(fp)) {
				//save pointer for strtok_r
				char * save;

				char * truncatedStrTok = strtok_r(argv[1], ":", &save);

				if (strstr(truncatedStrTok, "$PATH") != NULL) {
					char * envPath = getenv("PATH");
					strip(save);
					path = (char *) calloc(strlen(envPath) + strlen(save) + 2, sizeof(char));
					strcpy(path, envPath);
					strcat(path, ":");
					strcat(path,save);
				}
				else {
					strip(save);
					path = (char *) calloc(strlen(save) + 1, sizeof(char));
					strcat(path,save);
				}

				setenv("PATH", path, 1);
				//clean up the makealiasargs call
				cleanTypeAlias(buildAliasType_Args(argv));
			}
		}

	}
	else {
		//printf("Error opening file: %s.\n", strerror(errno));
		histcount = 100;
		histfilecount = 1000;
	}

	fclose(fp);

	printList(aliasList, printAliasType, 0);

	strcpy(s, "");

	/****************
	 * SETUP HISTORY
	 ****************/
	fp = fopen(".mssh_history", "rw");

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
		Node * lastHistory= retrieveNthLast(historyList, 0);

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

			/****************
			 * PREFORK SPECIAL CASE CODE
			 ****************/
			//special case: redirection
			checkForRedirection(s, &command, &inRedirect, &outRedirect);

			//special case: exclamation points
			if (*(command) == '!') {
				checkExclamations(&command, historyList);
			}

			//special case: aliases
			checkForAlias(&command, aliasList);

			/****************
			 * MAKEARGS FOR EXECUTION
			 ****************/
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
				res = execvp(argv[0], argv);


				//deal with bad result from exec
				if (res == -1) {
					//printf("Child didn't execute correctly.\n");
					exit(-1);
				}
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
				//special case: setting path
				else if (strstr(argv[0], "PATH") == argv[0]) {
					//free old path
					free(path);
					path = NULL;

					//save pointer for strtok_r
					char * save;

					//clear front of command (path=) away
					char * truncatedStrTok = strtok_r(argv[0], "=", &save);

					truncatedStrTok = strtok_r(NULL, ":", &save);

					char newPath[PATH_MAX];
					strcpy(newPath, "");

					int x = 0;
					while (truncatedStrTok != NULL) {
						if (x != 0) {
							strcat(newPath, ":");
							x++;
						}
						strip(save);

						if (strcmp(truncatedStrTok, "$PATH") == 0) {
							char *envPath = getenv("PATH");
							strcat(newPath, envPath);
						}
						else {
							strcat(newPath, truncatedStrTok);
						}


						truncatedStrTok = strtok_r(NULL, ":", &save);
					}

					path = (char *)calloc(strlen(newPath) + 1, sizeof(char));
					strcpy(path, newPath);
					setenv("PATH", path, 1);
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
	char * historyPath = (char *)calloc(strlen(startingDir) + strlen("/.mssh_history") + 1, sizeof(char));
	strcpy(historyPath, startingDir);
	strcat(historyPath, "/.mssh_history");

	fp = fopen(historyPath, "w");

	free(historyPath);
	historyPath = NULL;

	if (fp != NULL) {
		//clear contents of file
		ftruncate(fileno(fp), 0);

		rewind(fp);

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

	free(path);
	path = NULL;

	clearList(historyList, cleanTypeHistory);
   	free(historyList);
   	historyList = NULL;

	clearList(aliasList, cleanTypeAlias);
   	free(aliasList);
   	aliasList = NULL;

   	printf("Program Ended\n");

  	return 0;

}// end main

