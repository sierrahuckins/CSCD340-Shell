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

#define MAX 100
#define TRUE 1
#define FALSE 0

int main()
{
	int argc, histcount, histfilecount, linesfromhistory;
	int historyentriesadded = 0;
	int histfilelines = 0;
	char **argv = NULL, s[MAX], temp[MAX];
	char * path = NULL;
	int pathSet = FALSE;
	LinkedList * aliasList = linkedList();
	LinkedList * historyList = linkedList();
	char ** prePipe = NULL, ** postPipe = NULL;
	int preCount = 0, postCount = 0;
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
			if (argv[1] != NULL && !feof(fp)) {
				//save pointer for strtok_r
				char * save;

				path = (char *) calloc(strlen(argv[1]) + 1 - 6, sizeof(char));

				//we need to remove the front part
				char * truncatedStrTok = strtok_r(argv[1], ":", &save);

				strcpy(path, save);

				pathSet = TRUE;

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

	printList(aliasList, printAliasType);

	//close .msshrc
	fclose(fp);
	strcpy(s, "");

	//check if .mssh_history exists
	fp = fopen("/home/shuckins/.mssh_history", "r");

	if (fp != NULL) {
		//find out how many lines are in history
		fgets(s, MAX, fp);

		while (!feof(fp)) {
			if (strcmp(s, "") != 0)
				histfilelines++;
			fgets(s, MAX, fp);
		}

		//figure out how many lines in history we need to skip and skip those
		int skip = histfilelines - histcount;
		if (skip < 0)
			skip = 0;

		rewind(fp);

		int y;
		for (y = 0; y < skip; y++) {
			fgets(s, MAX, fp);
		}

		//read history entries into linked list
		int x;
		for (x = 0; x < histcount && x < histfilelines; x++) {
			if (strcmp(s, "") != 0) {
				fgets(s, MAX, fp);
				strip(s);

				argc = makehistoryargs(s, &argv);

				addLast(historyList, buildNode_Type(buildHistoryType_Args(argc, argv)));
			}
		}

		//going to use this variable to not double write some history entries later
		linesfromhistory = x;

	}
	//else if it doesn't exist, create it
	else {
		fp = fopen("/home/shuckins/.mssh_history", "w");
	}

	//close .mssh_history
	fclose(fp);

	printList(historyList, printHistoryType);

	//request command from user
  	printf("command?: ");
  	fgets(s, MAX, stdin);
  	strip(s);

	//handle command if it wasn't exit
  	while(strcmp(s, "exit") != 0) {
		argc = makehistoryargs(s, &argv);
		history * currentArgs = (history *)buildHistoryType_Args(argc, argv);

		Node * lastHistory= retrieveLast(historyList);

		//either add to history or clean up the args we've made if it already matches last history entry
		if (historyList->size == 0) {
			addLast(historyList, buildNode_Type(currentArgs));
			historyentriesadded++;
			printList(historyList, printHistoryType);
		}
		else if(compareHistory(currentArgs, lastHistory->data) != 0) {
			//makesure we're not over histcount
			if (historyList->size >= histcount)
				removeFirst(historyList, cleanTypeHistory);
			addLast(historyList, buildNode_Type(currentArgs));
			historyentriesadded++;
			printList(historyList, printHistoryType);
		}
		else {
			//clean up the makehistoryargs call
			cleanTypeHistory(currentArgs);
		}

		//check for pipe
		int pipes = containsPipe(s);

		//split if necessary
		if (pipes > 0) {
			//split off prepipe
			prePipe = parsePrePipe(s, &preCount);
			argv = prePipe;

			//check if prePipe is an alias and update if necessary
			if (preCount == 1) {
				checkAliasArgs = (alias *)buildAliasType_Args(argv);
				Node * prePipeNode = buildNode_Type(checkAliasArgs);

				prePipeNode = findInList(aliasList, prePipeNode, compareAlias);
				checkAliasArgs = (alias *)prePipeNode->data;
				preCount = makeargs(checkAliasArgs->argv[1], &prePipe);
			}

			//split off post pipe
			postPipe = parsePostPipe(s, &postCount);
			argv = postPipe;

			//check if postPipe is an alias and update if necessary
			if (postCount == 1) {
				checkAliasArgs = (alias *)buildAliasType_Args(argv);
				Node * postPipeNode = buildNode_Type(checkAliasArgs);

				postPipeNode = findInList(aliasList, postPipeNode, compareAlias);
				checkAliasArgs = (alias *)postPipeNode->data;
				postCount = makeargs(checkAliasArgs->argv[1], &postPipe);
			}

			//do the pipe!
			pipeIt(prePipe, postPipe);
			clean(preCount, prePipe);
			clean(postCount, postPipe);
		}
		//else handle like a normal command
		else {
			//check if command is an alias and update if necessary
			argc = makeargs(s, &argv);

			if (argc == 1) {
				checkAliasArgs = (alias *)buildAliasType_Args(argv);
				Node * commandNode = buildNode_Type(checkAliasArgs);

				commandNode = findInList(aliasList, commandNode, compareAlias);
				checkAliasArgs = (alias *)commandNode->data;
				argc = makeargs(checkAliasArgs->argv[1], &argv);
			}

			//fork a child to execute command
			pid_t pid;
			int res, status;
			pid = fork();

			if(pid != 0) {
				waitpid(pid, &status, 0);
			}

			else {
				int result = execvp(argv[0], argv);

				//deal with bad result from exec
				if (result == -1) {
					exit(-1);
				}
			}
		}


		//printList(historyList, printHistoryType);

		printf("command?: ");
	  	fgets(s, MAX, stdin);
		strip(s);

  	}// end while

	//write out to .msshrc_history
	fp = fopen("/home/shuckins/.mssh_history", "r");

	if (fp != NULL) {
		//create temp file to write to
		tempFile = fopen("/home/shuckins/temp", "w");

		//figure out how many lines in history we need to skip and skip those
		int totalHistory = histfilelines + historyList->size;
		int skip = totalHistory - histfilecount;
		if (skip < 0)
			skip = 0;

		int w;
		for (w = 0; w < skip; w++) {
			fgets(s, MAX, fp);
		}

		//read history entries into templist
		rewind(fp);

		int x;
		for (x = 0; x < histcount && x < histfilelines; x++){
			fgets(s, MAX, fp);
			fputs(s, tempFile);
		}

		//determine if we need to skip some historylist entries (that were already in list)
		skip = linesfromhistory;
		if (historyList->size == histcount)
			skip = skip - historyentriesadded;
		if (skip < 0)
			skip = 0;

		int y;
		for (y = 0; y < skip; y++) {
			removeFirst(historyList, cleanTypeHistory);
		}

		//read historylist into templist
		for (y = 0; y <= historyList->size; y++) {
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
		fclose(tempFile);

		//delete old file
		remove("/home/shuckins/.mssh_history");

		//rename temp file
		rename("/home/shuckins/temp", "/home/shuckins/.mssh_history");
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







