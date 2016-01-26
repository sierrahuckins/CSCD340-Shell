#include "myUtils.h"

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

void checkForAlias(char * s, char ** command, LinkedList * aliasList) {
    Node * curr = aliasList->head->next;

    while (curr != NULL) {
        alias * currAlias = curr->data;

        if (strcmp(s, currAlias->argv[0]) == 0) {
            free(*command);
            *command = NULL;

            *command = (char *)calloc(strlen(currAlias->argv[1]) + 1, sizeof(char));
            strcpy(*command, currAlias->argv[1]);
            break;
        }

        curr = curr->next;
    }

}

void checkForAliasToRemove(char * s, LinkedList * aliasList) {
    Node * curr = aliasList->head->next;

    while (curr != NULL) {
        alias * currAlias = curr->data;

        if (strcmp(s, currAlias->argv[0]) == 0) {
            removeItem(aliasList, curr, cleanTypeAlias, compareAlias);
            break;
        }

        curr = curr->next;
    }

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
        char * save = NULL;

        //both redirects are present
        if (strstr(s, "<") != NULL && strstr(s, ">") != NULL) {

            //figure out which redirect is first and handle accordingly
            if (strstr(inRedirect, ">") != NULL) {
                //pull off command token
                char * tempStrTok = strtok_r(newStr, "<", &save);

                //dynamically allocate new memory
                *command = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
                strcpy(*command, tempStrTok);

                //pull off in token
                tempStrTok = strtok_r(NULL, ">", &save);

                //dynamically allocate new memory
                *redirectInPath = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
                strcpy(*redirectInPath, tempStrTok);

                //pull out token from save
                *redirectOutPath = (char *) calloc(strlen(save) + 1, sizeof(char));
                strcpy(*redirectOutPath, save);
            }
            else {
                //pull off command token
                char * tempStrTok = strtok_r(newStr, ">", &save);

                //dynamically allocate new memory
                *command = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
                strcpy(*command, tempStrTok);

                //pull off in token
                tempStrTok = strtok_r(NULL, "<", &save);

                //dynamically allocate new memory
                *redirectOutPath = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
                strcpy(*redirectOutPath, tempStrTok);

                //pull out token from save
                *redirectInPath = (char *) calloc(strlen(save) + 1, sizeof(char));
                strcpy(*redirectInPath, save);
            }
        }
        //only in redirect is present
        else if (strstr(s, "<") != NULL) {
            //pull off command token
            char * tempStrTok = strtok_r(newStr, "<", &save);

            //dynamically allocate new memory
            *command = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
            strcpy(*command, tempStrTok);

            //pull off in token
            tempStrTok = strtok_r(NULL, ">", &save);

            //dynamically allocate new memory
            *redirectInPath = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
            strcpy(*redirectInPath, tempStrTok);
        }
        //only out redirect is present
        else if (strstr(s, ">") != NULL) {
            //pull off command token
            char * tempStrTok = strtok_r(newStr, ">", &save);

            //dynamically allocate new memory
            *command = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
            strcpy(*command, tempStrTok);

            //pull off in token
            tempStrTok = strtok_r(NULL, "<", &save);

            //dynamically allocate new memory
            *redirectOutPath = (char *) calloc(strlen(tempStrTok) + 1, sizeof(char));
            strcpy(*redirectOutPath, tempStrTok);
        }

        //free dynamically allocated memory
        free(newStr);
        newStr = NULL;
    }
}
