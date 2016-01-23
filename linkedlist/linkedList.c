#include "linkedList.h"

/**
 * @brief The so called "constructor" for the linked list
 *
 * The linked list constructor builds a non circular linked list that
 * contains a dummy head not.  This is meant to be similar to a Java
 * linked list constructor.
 *
 * @return LinkedList * - The linked list pointer that contains a dummy head node but is not circular.
 */
LinkedList * linkedList() {
	Node * head = (Node *)calloc(1, sizeof(Node));

	LinkedList * list = (LinkedList *)calloc(1,sizeof(LinkedList));

	list->head = head;

	return list;
}


/**
 * @brief The add last function for the linked list
 *
 * Appends the specified node to the end of this list
 *
 * @param theList - The specified linked list
 * @param nn - The node to be added
 *
 * @warning - The passed in LinkedList * theList is checked - exit(-99) if NULL
 * @warning - The passed in Node * nn is checked - exit(-99) if NULL
 */
void addLast(LinkedList * theList, Node * nn) {
	//check if passed in list is null
	if (theList == NULL)
		exit(-99);

	//check if passed in node is null
	else if (nn == NULL) 
		exit(-99);

	//passed preconditions, continue with function
	else {
		//create curr pointer and move it to end of list	
		Node * curr = theList->head;
	
		while (curr->next != NULL) {
			curr = curr->next;
		}

		//add to end of list
		curr->next = nn;

		//increase list size
		theList->size = theList->size + 1;
	}
}


/**
 * @brief The add first function for the linked list
 *
 * Appends the specified node to the beginning of this list
 *
 * @param theList - The specified linked list
 * @param nn - The node to be added
 *
 * @warning - The passed in LinkedList * theList is checked - exit(-99) if NULL
 * @warning - The passed in Node * nn is checked - exit(-99) if NULL
 */
void addFirst(LinkedList * theList, Node * nn) {
	//check if passed in list is null
	if (theList == NULL)
		exit(-99);

	//check if passed in node is null
	else if (nn == NULL) 
		exit(-99);

	//passed preconditions, continue with function
	else {	
		Node * head = theList->head;

		nn->next = head->next;
		head->next = nn;

		//increase list size
		theList->size = theList->size + 1;
	}
}

void removeFirst(LinkedList * theList, void (*removeData)(void *)) {
	//check if passed in list is null
	if (theList == NULL)
		exit(-99);

//passed preconditions, continue with function
	else {
		Node * head = theList->head;
		Node * next = head->next->next;
		Node * removed = head->next;

		head->next = next;

		removeData(removed->data);
		free(removed);
		removed = NULL;

		theList->size = theList->size--;

	}
}

void * retrieveLast(LinkedList * theList) {
	//check if passed in list is null
	if (theList == NULL)
		exit(-99);

		//passed preconditions, continue with function
	else {
		//create curr pointer and move it to end of list
		Node * curr = theList->head;

		while (curr->next != NULL) {
			curr = curr->next;
		}

		return curr;
	}
}


/**
 * @brief The remove item function for the linked list
 *
 * Removes the first occurrence of the specified element from this list, if it
 * is present.  If this list does not contain the element, it is unchanged.
 *
 * @param theList - The specified linked list
 * @param nn - The node to be added
 * @param *removeData - The function pointer for freeing the specific data type
 * @param *compare - The compare function to compare specific data type
 *
 * @warning - The passed in LinkedList * theList is checked - exit(-99) if NULL
 * @warning - The passed in Node * nn is checked - exit(-99) if NULL
 */
void removeItem(LinkedList * theList, Node * nn, void (*removeData)(void *), int (*compare)(const void *, const void *)) {
    void * toRemove = nn->data;
    
	//check if passed in list is null
	if (theList == NULL)
		exit(-99);

	//check if passed in node is null
	else if (nn == NULL) 
		exit(-99);

	//passed preconditions, continue with function
	else {
		Node * curr = theList->head;
		Node * temp;

		//if list is empty
		if (curr->next == NULL) 
			printf("List empty. No items removed.");
		
		else {
		    Node * prev = theList->head;
		    curr = curr->next;

		    //cycle through list and compare
		    while (curr != NULL) {
				//if data is equal to given node, then delete and end
				if (compare(curr->data, toRemove) == 0) {
					temp = curr;
					prev->next = curr->next;
					curr = curr->next;

					theList->size = theList->size - 1;

					removeData(temp->data);
					free(temp);
					temp = NULL;
				}
				//not the node we're looking for, so move on
				else {
					prev = prev->next;
					curr = curr->next;
				}
		    }
		}
	}

	//cleanup given node
	removeData(toRemove);
	free(nn);
	nn = NULL;
}


/**
 * @brief Empties the list and its contents
 *
 * Removes all of the elements from this list.  The list will be empty after the function completes
 *
 * @param theList - The specified linked list
 * @param *removeData - The function pointer for freeing the specific data type
 * @param *compare - The compare function to compare specific data type
 *
 * @warning - The passed in LinkedList * theList is checked - if NULL nothing happens
 */
void clearList(LinkedList * theList, void (*removeData)(void *)) {
	//check if passed in list is null
	if (theList == NULL)
		exit(-99);

	//passed preconditions, continue with function
	else {
		Node * curr = theList->head->next;
		Node * temp;
		void * dataToDelete;

		while (curr != NULL) {
			temp = curr;
			curr = curr->next;

			removeData(temp->data);
			free(temp); 
			temp = NULL;
		}

		free(theList->head);
		theList->head = NULL;

		theList->size = 0;
	}
}


/**
 * @brief Prints the contents of this linked list
 *
 * Prints the contents, if there are any of this linked list.  If the list is NULL
 * or empty, "Empty List" is printed.
 *
 * @param theList - The specified linked list
 * @param *convert - The function pointer for printing the specific data type
 *
 * @warning - The passed in LinkedList * theList is checked - if NULL "Empty List" is printed
 */
void printList(const LinkedList * theList, void (*convertData)(void *)) {
	//check if passed in list is null
	if (theList == NULL)
		printf("Empty List");

	//passed preconditions, continue with function
	else {
		Node * curr = theList->head->next;

		while (curr != NULL) {
			convertData(curr->data);
			curr = curr->next;
		}

		printf("\n");
	}
}
