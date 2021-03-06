#include "listUtils.h"

/**
 * @brief Builds a node that contains a pointer to the specific data type.
 *
 * The buildNode function builds a node for the linked list. The node's void *
 * data will refer to the specific data type. The specific data type is built
 * by calling the appropriate specific data function, which is called via
 * function pointer.
 *
 * @param fin - The FILE * used to read the data from the file
 * @param *buildData - The function pointer to build an object of the specific data type
 * @return Node * - Representing a node for the linked list containing the specific data type.
 *
 * @note - The first parameter FILE * fin is not used in the function.  It is entirely
 * used as a pass through parameter for the function pointer which is passed a FILE * also.
 *
 * @warning - Since FILE *fin is a pass through it is not checked.
 */
Node * buildNode(FILE * fin, void *(*buildData)(FILE * in) ) {
	Node * newNode = (Node *)calloc(1,sizeof(Node));
	
	newNode->data = buildData(fin);
	
	return newNode;
}


/**
 * @brief Builds a node that contains a call to a specific type.
 *
 * The buildNode_Type function builds a node for the linked list. The node's void *
 * data will refer to the specific data type. The specific data type is built
 * by calling the appropriate specific data function, which is called via
 * function pointer.
 *
 * @param *passedIn - The void * for the data type being created
 * @return Node * - Representing a node for the linked list containing the specific data type.
 */
Node * buildNode_Type(void * passedIn) {
	Node * newNode = (Node *)calloc(1,sizeof(Node));
	
	newNode->data = passedIn;
	
	return newNode;
}


/**
 * @brief Sorts the linked list.
 *
 * The sort function resides here because sorting a linked list
 * is not considered an integral function within the required
 * linked list functions.
 *
 * @param theList - The linked list  * representing the list
 * @param *compare - The specific data type compare function as a function pointer
 *
 * @warning - The passed in LinkedList * theList is checked - exit(-99) if NULL
 * @warning - The theList-> size is checked and if the list contains 0 or 1 element then the function
 * does not attempt to sort the list.
 */
void sort(LinkedList * theList, int (*compare)(const void *, const void *)) {
	//check if passed in list is null
	if (theList == NULL)
		exit(-99);

	//check if list is of size 0 or 1
	else if (theList->size != 0 && theList->size != 1) {
		//passed preconditions, continue with function
		Node * i, * j, * smallest;
		void * temp;

		for(i = theList->head->next; i != NULL; i = i->next) {
			smallest = i;

			for(j = i; j != NULL; j = j->next) {
			    if (compare(smallest->data,j->data) > 0) {
				smallest = j;
			    }
			}

			temp = i->data;
			i->data = smallest->data;
			smallest->data = temp;
		}
	}
}


/**
 * @brief Builds an initial linked list.
 *
 * The build list creates an initial linked list by reading from
 * the file, calling the buildNode function and then adds that node
 * into the list by calling addFirst.
 *
 * @param theList - The linked list  * representing the list
 * @param total - The total items to be placed into the list
 * @param fin - The FILE * used to read the data from the file
 * @param *buildData - The function pointer to call the specific function
 * to build the appropriate data type.
 *
 * @note - The parameter FILE * fin is not used in the function.  It is entirely
 * used as a pass through parameter for the function pointer which is also passed a FILE *.
 *
 * @warning - The passed in LinkedList * theList is checked - exit(-99) if NULL
 * @warning - The passed in int total is checked - exit(-99) if <= 0
 * @warning - Since FILE *fin is a pass through it is not checked.
 */
void buildList(LinkedList * myList, int total, FILE * fin, void * (*buildData)(FILE * in))  {
	//check if passed in list is null
	if (myList == NULL)
		exit(-99);

	//check if passed in node is null
	else if (total <= 0) 
		exit(-99);	

	//passed preconditions, continue with function
	else {

		    
		//continue for total number recrds
		int x;
		for (x = 0; x < total; x++)
		{
			//create void * for data
			void * temp = buildData(fin);

			Node * newNode = (Node *)calloc(1,sizeof(Node));

			newNode->data = temp;

			addFirst(myList, newNode);
		}

		myList->size = total;
	}
}
