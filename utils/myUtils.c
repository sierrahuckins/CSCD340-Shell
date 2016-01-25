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
            *command = (char *)calloc(strlen(currAlias->argv[1]) + 1, sizeof(char));

            strcpy(*command, currAlias->argv[1]);
            break;
        }

        curr = curr->next;
    }

}

void checkForRedirection(char * s, char ** command) {

}
