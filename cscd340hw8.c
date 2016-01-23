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

#define MAX 100
#define TRUE 1
#define FALSE 0

int main()
{
	int argc, histcount, histfilecount;
	char **argv = NULL, s[MAX], temp[MAX];
	char * path;
	int pathSet = FALSE;
	LinkedList * aliasList = linkedList();
	LinkedList * historyList = linkedList();
	char ** prePipe = NULL, ** postPipe = NULL;
	int preCount = 0, postCount = 0;
	
	//check if .msshrc exists and setup variables if it does
	FILE * fp = fopen("/home/shuckins/.msshrc", "r");

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

	//check if .mssh_history exists
	fp = fopen("/home/shuckins/.mssh_history", "r");

	if (fp != NULL) {
		fgets(s, MAX, fp);
		strip(s);

		while(!feof(fp)) {
			argc = makehistoryargs(s, &argv);

			addLast(historyList, buildNode_Type(buildHistoryType_Args(argc, argv)));

			fgets(s, MAX, fp);
			strip(s);
		}

	}

	printList(historyList, printHistoryType);

	//close .mssh_history
	fclose(fp);

	
  	printf("command?: ");
  	fgets(s, MAX, stdin);
  	strip(s);

  	while(strcmp(s, "exit") != 0) {
		argc = makehistoryargs(s, &argv);
		void * currentArgs = buildHistoryType_Args(argc, argv);

		Node * lastHistory= retrieveLast(historyList);

		if(compareHistory(currentArgs, lastHistory->data) != 0) {
			//makesure we're not over histcount
			if (historyList->size >= histcount)
				removeFirst(historyList, cleanTypeHistory);
			addLast(historyList, buildNode_Type(currentArgs));
		}
		else {
			//clean up the makehistoryargs call
			cleanTypeHistory(currentArgs);
		}

		//check for pipe
		int pipes = containsPipe(s);

		//split if necessary
		if (pipes > 0) {
			prePipe = parsePrePipe(s, &preCount);
			postPipe = parsePostPipe(s, &postCount);
		}


		printList(historyList, printHistoryType);

		printf("command?: ");
	  	fgets(s, MAX, stdin);
		strip(s);

  	}// end while

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







