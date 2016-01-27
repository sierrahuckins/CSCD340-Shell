#include "myUtils.h"
#include "../linkedlist/listUtils.h"

void strip(char *array) {
    if (array == NULL) {
        perror("array is null");
        exit(-99);
    }// end if

    int len = strlen(array), x = 0;

    while (array[x] != '\0' && x < len) {
        if (array[x] == '\r')
            array[x] = '\0';

        else if (array[x] == '\n')
            array[x] = '\0';

        x++;
    }// end while
}

/****************************
 * Code (as used by Linux Kernel) via
 * http://lxr.free-electrons.com/source/lib/string.c?v=2.6.32#L348
 *
 * Removes leading and
 * trailing whitespaces.
 ***************************/
char *strstrip(char *s)
{
    size_t size;
    char *end;

    size = strlen(s);

    if (!size)
        return s;

    end = s + size - 1;
    while (end >= s && isspace(*end))
        end--;
    *(end + 1) = '\0';

    while (*s && isspace(*s))
        s++;

    return s;
}

void checkExclamations (char ** command, LinkedList * historyList) {
    //get last history
    Node *lastHistory;

    if (*(*command + 1) == '!') {
        lastHistory = retrieveNthLast(historyList, 1);
    }
    else {
        int val = 1;
        char * p = *command;
        while(*p) {
            if (isdigit(*p))
                val = (int)strtol(p, &p, 10);
            else
                p++;
        }

        lastHistory = retrieveNth(historyList, val);
    }

    history *lastCommand = lastHistory->data;

    char tempCommand[MAX];

    strcpy(tempCommand, lastCommand->argv[0]);

    int x;
    for (x = 1; x < lastCommand->argc; x++) {
        strcat(tempCommand, " ");
        strcat(tempCommand, lastCommand->argv[x]);
    }

    free(*command);
    *command = NULL;

    *command = (char *)calloc(strlen(tempCommand) + 1, sizeof(char));
    strcpy(*command, tempCommand);
}

void checkForAlias(char ** command, LinkedList * aliasList) {
    Node * curr = aliasList->head->next;

    while (curr != NULL) {
        alias * currAlias = curr->data;

        if (strcmp(*command, currAlias->argv[0]) == 0) {
            free(*command);
            *command = NULL;

            *command = (char *)calloc(strlen(currAlias->argv[1]) + 1, sizeof(char));
            strcpy(*command, currAlias->argv[1]);
            break;
        }

        curr = curr->next;
    }

}

void checkForAliasToRemove(char ** argv, LinkedList * aliasList) {


}

void checkForRedirection(char * s, char ** command, char ** redirectInPath, char ** redirectOutPath) {
    if (s == NULL) {
        exit(-1);
    }
    else {
        char * inRedirect = strstr(s, "<");
        char * outRedirect = strstr(s, ">");

        //copy string for tokenizing
        char * newStr = (char *) calloc(strlen(s) + 1, sizeof(char));
        strcpy(newStr, s);
        s = strstrip(s);
        char * save = NULL;

        //both redirects are present
        if (strstr(s, "<") != NULL && strstr(s, ">") != NULL) {

            //figure out which redirect is first and handle accordingly
            if (strstr(inRedirect, ">") != NULL) {
                //pull off command token
                char * tempStrTok = strstrip(strtok_r(newStr, "<", &save));

                //dynamically allocate new memory
                *command = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
                strcpy(*command, tempStrTok);

                //pull off in token
                tempStrTok = strstrip(strtok_r(NULL, ">", &save));

                //dynamically allocate new memory
                *redirectInPath = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
                strcpy(*redirectInPath, tempStrTok);

                //pull out token from save
                *redirectOutPath = (char *) calloc(strlen(save) + 1, sizeof(char));
                strcpy(*redirectOutPath, strstrip(save));
            }
            else {
                //pull off command token
                char * tempStrTok = strstrip(strtok_r(newStr, ">", &save));

                //dynamically allocate new memory
                *command = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
                strcpy(*command, tempStrTok);

                //pull off in token
                tempStrTok = strstrip(strtok_r(NULL, "<", &save));

                //dynamically allocate new memory
                *redirectOutPath = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
                strcpy(*redirectOutPath, tempStrTok);

                //pull out token from save
                *redirectInPath = (char *) calloc(strlen(save) + 1, sizeof(char));
                strcpy(*redirectInPath, strstrip(save));
            }
        }
        //only in redirect is present
        else if (strstr(s, "<") != NULL) {
            //pull off command token
            char * tempStrTok = strstrip(strtok_r(newStr, "<", &save));

            //dynamically allocate new memory
            *command = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
            strcpy(*command, tempStrTok);

            //pull off in token
            tempStrTok = strstrip(strtok_r(NULL, ">", &save));

            //dynamically allocate new memory
            *redirectInPath = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
            strcpy(*redirectInPath, tempStrTok);
        }
        //only out redirect is present
        else if (strstr(s, ">") != NULL) {
            //pull off command token
            char * tempStrTok = strstrip(strtok_r(newStr, ">", &save));

            //dynamically allocate new memory
            *command = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
            strcpy(*command, tempStrTok);

            //pull off in token
            tempStrTok = strstrip(strtok_r(NULL, "<", &save));

            //dynamically allocate new memory
            *redirectOutPath = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
            strcpy(*redirectOutPath, tempStrTok);
        }
        //else no redirects
        else {
            //dynamically allocate new memory
            *command = (char *) calloc(strlen(s) + 1, sizeof(char));
            strcpy(*command, s);
        }

        //free dynamically allocated memory
        free(newStr);
        newStr = NULL;
    }
}
