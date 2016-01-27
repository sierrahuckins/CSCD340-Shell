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
	char *inRedirect = NULL;
	char *outRedirect = NULL;

	/****************
	 * HANDLE .MSSHRC
	 ****************/

	fp = fopen(".msshrc", "r");
	startingDir=getcwd(tempPath, PATH_MAX);

	if (fp != NULL) {

		/****************
		 * GET HIST VARIABLES
		 ****************/

		//get histcount from file
		histcount = getHistFileCount(fp, s);

		//get histfilecount from file
		histfilecount = getHistFileCount(fp, s);

		/****************
		 * GET ALIASES
		 ****************/

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

		fclose(fp);

	}
	else {
		//printf("Error opening file: %s.\n", strerror(errno));
		histcount = 100;
		histfilecount = 1000;
	}

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
		 * MAKE HISTORY SWITCHES
		 ****************/

		if (s[0] == '!') {
			//get last history
			Node *lastHistory;
			int val;

			if (s[1] == '!') {
				lastHistory = retrieveNthLast(historyList, 1);
			}
			else {
				char * p = s;
				p++;
				strcpy(s, p);
				val = (int) strtol(s, &p, 10);

				lastHistory = retrieveNth(historyList, val);
			}

			history *lastCommand = lastHistory->data;

			strcpy(s, lastCommand->argv[0]);

			int x;
			for (x = 1; x < lastCommand->argc; x++) {
				strcat(s, " ");
				strcat(s, lastCommand->argv[x]);
			}
		}

		/****************
		 * HANDLE PATH COMMAND
		 ****************/
		if (strstr(s, "PATH=") == s) {
			//free old path
			free(path);
			path = NULL;

			char sCopy[PATH_MAX];
			strcpy(sCopy, s);

			//save pointer for strtok_r
			char * save;

			//clear front of command (path=) away
			char * truncatedStrTok = strtok_r(sCopy, "=", &save);

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

		/****************
		 * DETERMINE IF PIPED
		 ****************/

		int pipes = containsPipe(s);

		if (pipes > 0) {
			//split into left and right strings
			char * leftPipeString = NULL;
			char * rightPipeString = NULL;
			char * leftCommand, * rightCommand;
			splitForPipe(s, &leftCommand, &rightCommand);

			/****************
			 * PREFORK/PREARG SPECIAL CASES
			 ****************/

			//special case: redirection
			checkForRedirection(leftCommand, &leftPipeString, &inRedirect, &outRedirect);
			checkForRedirection(rightCommand, &rightPipeString, &inRedirect, &outRedirect);

			//special case: is an alias
			checkForAlias(&leftPipeString, aliasList);
			checkForAlias(&rightPipeString, aliasList);

			//special case: create alias
			if (strstr(leftPipeString, "alias ") == leftPipeString) {
				//make new alias style args
				argc = makealiasargs(s, &argv);

				//add to alias list
				addLast(aliasList, buildNode_Type(buildAliasType_Args(argv)));

				//printList(aliasList, printAliasType, 0);

				argv = NULL;
			}
			if (strstr(rightPipeString, "alias ") == rightPipeString) {
				//make new alias style args
				argc = makealiasargs(s, &argv);

				//add to alias list
				addLast(aliasList, buildNode_Type(buildAliasType_Args(argv)));

				//printList(aliasList, printAliasType, 0);

				argv = NULL;
			}

			/*

			//special case: unalias
			if (strstr(leftPipe, "unalias ") == leftPipe) {
				printf("Unaliasing not currently supported.");

				make new alias style args
				argc = makeargs(s, &argv);
				char temp[MAX];
				strcpy(temp, argv[1]);

				clean(argc, argv);
				argv = NULL;

				argc = 2;
				argv = (char **)calloc(3, sizeof(char *));
				argv[0] = (char *) calloc(strlen(temp) + 1, sizeof(char));
				strcpy(argv[0], temp);
				argv[1] = (char *) calloc(strlen("TEST") + 1, sizeof(char));
				strcpy(argv[0], "TEST\0");
				argv[3] = '\0';

				Node * removeNode = buildNode_Type(buildAliasType_Args(argv));

				removeItem(aliasList, removeNode, cleanTypeAlias, compareAlias);
				printList(aliasList, printAliasType, 0);

				//clean up the makeargs call
				argv = NULL;
			}
*/

			/****************
			 * MAKEARGS FOR EXECUTION
			 ****************/

			char ** leftPipe, ** rightPipe;
			int leftCount = 0, rightCount = 0;
			leftCount = makeargs(leftPipeString, &leftPipe);
			rightCount = makeargs(rightPipeString, &rightPipe);

			//special case:cd
			if (strcmp(leftPipe[0], "cd") == 0) {
				chdir(leftPipe[1]);
			}
			if (strcmp(rightPipe[0], "cd") == 0) {
				chdir(rightPipe[1]);
			}

			/****************
			 * PIPEIT WILL GO HERE
			 ****************/

			pipeIt(leftPipe, rightPipe, &inRedirect, &outRedirect, historyList, histcount);

			//free my pipe string
			free(leftPipeString);
			leftPipeString = NULL;
			free(rightPipeString);
			rightPipeString = NULL;

			free(leftCommand);
			leftCommand = NULL;
			free(rightCommand);
			rightCommand = NULL;

			clean(leftCount, leftPipe);
			clean(rightCount, rightPipe);

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
		/****************
		 * HANDLE SINGLE COMMAND
		 ****************/

		else {
			char *command;

			/****************
			 * PREFORK/PREARG SPECIAL CASES
			 ****************/
			//special case: redirection
			checkForRedirection(s, &command, &inRedirect, &outRedirect);
			command = strstrip(command);

			//special case: is an alias
			checkForAlias(&command, aliasList);

			//special case: create alias
			if (strstr(command, "alias ") == command) {
				//make new alias style args
				argc = makealiasargs(s, &argv);

				//add to alias list
				addLast(aliasList, buildNode_Type(buildAliasType_Args(argv)));

				printList(aliasList, printAliasType, 0);

				argv = NULL;

			}

			//special case: unalias
			if (strstr(command, "unalias ") == command) {
				printf("Unaliasing not currently supported.");

				/*//make new alias style args
				argc = makeargs(s, &argv);
				char temp[MAX];
				strcpy(temp, argv[1]);

				clean(argc, argv);
				argv = NULL;

				argc = 2;
				argv = (char **)calloc(3, sizeof(char *));
				argv[0] = (char *) calloc(strlen(temp) + 1, sizeof(char));
				strcpy(argv[0], temp);
				argv[1] = (char *) calloc(strlen("TEST") + 1, sizeof(char));
				strcpy(argv[0], "TEST\0");
				argv[3] = '\0';

				Node * removeNode = buildNode_Type(buildAliasType_Args(argv));

				removeItem(aliasList, removeNode, cleanTypeAlias, compareAlias);
				printList(aliasList, printAliasType, 0);

				//clean up the makeargs call
				argv = NULL;*/
			}

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
				 * HANDLE EXE COMMANDS
				 ****************/
				//special case: history
				if (strcmp(argv[0], "history") == 0) {
					printList(historyList, printHistoryType, (historyList->size - histcount));
				}

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

				//special case:cd
				if (strcmp(argv[0], "cd") == 0) {
					chdir(argv[1]);
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

