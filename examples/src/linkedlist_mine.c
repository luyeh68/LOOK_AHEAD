// to create a linked_list for the g-code parser: this list is a collection of
// copies of c-structure where each copy of the structure contains a pointer to
// a previous copy and one to the next one
//   _     _       _            _   _ _     _
//  | |   (_)_ __ | | _____  __| | | (_)___| |_
//  | |   | | '_ \| |/ / _ \/ _` | | | / __| __|
//  | |___| | | | |   <  __/ (_| | | | \__ \ |_
//  |_____|_|_| |_|_|\_\___|\__,_| |_|_|___/\__|

// The list is a chain of structures linked with twin pointers:
//         ┌───┬──────┐     ┌───┬──────┐     ┌───┬──────┐
// NULL ◄──┤PRV│      │◄────┤ P │      │◄────┤ P │      │
//         ├───┘  ┌───┤     ├───┘  ┌───┤     ├───┘  ┌───┤
//         │      │NXT├────►│      │ N ├────►│      │ N ├──► NULL
//         └──────┴───┘     └──────┴───┘     └──────┴───┘
// So, strictly speaking we are implementing a doubly linked list
// A singly linked list would be simpler, but it can only be travelled
// forward.

/*The very first element in the list has NULL as pointer to previous element
while the very last list element has NULL  as pointer to the next element*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------
// structs in C

/*
struct myStruct{

}; */

// struct myStruct *structName; //to use the (struct myStruct) = type of
// variable structName or also without *

// IS BORING TO TYPE struct myStruct anytime so:

// typedef int Nicolosh; // my own name as integer variables type definition

/*
typedef struct myStruct{

} myStruct_t; // new struct type!
*/

// so by doing so we can omit the name myStruct and write:

/*
typedef struct{

} myStruct_t; // new struct type!
*/

// ----------------------------------------------------------------

// structure representing an object in the list
typedef struct element {
  char *id;
  // double value;
  // int ary[1024];
  struct element *elementPrev;
  struct element *elementNext;
} element_t; // struct used as an element of a linked list

// structure representing a list itself
typedef struct {
  element_t *VFelement, *VLelement;
  size_t length; // n° of list elements
} list_t;

// creating a new element_t element given its ID
element_t *elementNew(const char *id) {
  element_t *newElement = malloc(sizeof(element_t));
  memset(newElement, 0, sizeof(element_t));
  newElement->id = malloc(strlen(id) + 1);
  strncpy(newElement->id, id, strlen(id));
  return newElement;
}

void element_free(element_t *element) {
  free(element->id);
  free(element);
}

// --------------------------------------- METHODS --------------------------

// creates a new list_t and return a pointer to the new list_t: also creates a
// new list with a newly created element with a given ID
list_t *list_new(const char *id) {
  list_t *list = malloc(sizeof(list_t));
  // memset(list, 0, sizeof(list_t)); //set the memory to 0 in all the bytes
  // representing the memory location list (not to random numbers!)

  // creating a new element
  element_t *e = elementNew(id);

  // list fields inizialitation
  list->VFelement = e;
  list->VLelement = e;
  list->length = 1;

  return list;
}

// appending at THE END an element_t *e to an existing list
void list_append_element(list_t *list, element_t *e) {
  // updating the current next Element
  list->VLelement->elementNext = e;
  e->elementPrev = list->VLelement;
  list->VLelement = e;
  e->elementNext = NULL;
  list->length++;
}

// like a constructor for an element with that given ID and append that element
// to the list
element_t *list_append(list_t *list, const char *id) {
  element_t *e = elementNew(id);
  list_append_element(list, e);
  return e; // pointer to the newly created element_t e
}

// inserting a new element (e) after a really existing element or specific
// element with id = idAfter
void list_insert_element(list_t *list, element_t *e, const char *idAfter) {
  element_t *currentElement = list->VFelement; // first element in the list

  do {
    if (strcmp(currentElement->id, idAfter) == 0) // 2 strings are EQUAL
    {
      e->elementPrev = currentElement;
      e->elementNext = currentElement->elementNext;
      currentElement->elementNext->elementPrev = e;
      currentElement->elementNext = e;

      list->length++;
      break;
    }

  } while (
      (currentElement =
           currentElement->elementNext)); // != NULL: until there is another
                                          // element in the list or until e =
                                          // NULL is find in the list (listEND)
}

// creating and inserting a new element after a given id = idAfter: id is identifying the
// new element to be inserted
element_t *list_insert(list_t *list, const char *id, const char *idAfter) {
  element_t *newElement = elementNew(id);
  list_insert_element(list, newElement, idAfter);
  return newElement;
}

// method for removing an element with a given ID
void list_delete(list_t *list, const char *id) {
  element_t *iterator = list->VFelement;
  do {
    if (strcmp(iterator->id, id) == 0) // strings equal -->delete the element
    {
      iterator->elementPrev->elementNext = iterator->elementNext;
      iterator->elementNext->elementPrev = iterator->elementPrev;
      element_free(iterator);
      list->length--;
      break;
    }
  } while ((iterator = iterator->elementNext)); // != NULL
}

// free memory allocated for the list
void list_free(list_t *list) {

  element_t *iterator = list->VFelement;
  element_t *next = iterator->elementNext;
  
  do {
    element_free(iterator);
  } while ((iterator = next)); // != NULL
  free(list);
}

typedef enum { ASC, DESC } loop_order_t;

typedef void (*loop_func_t)(
    const element_t *e, loop_order_t order,
    void *userData); // SYNTAX FOR CREATING A TYPE DESCRIBING A FUNCTION typically implemented by the user (possibly us)

/* ------------------ CALL BACK ----------------- (function automatically called
 * by another function) */

// for operating on all the list elements (i.e each row of g-code at a time) in
// a given order, calling func over each element in turn
void list_loop(list_t *list, loop_func_t func, loop_order_t order,
               void *userData) {
  element_t *iterator;
  // start from beginning to end according order
  if (order == ASC)
    iterator = list->VFelement;
  else
    iterator = list->VLelement;

  // loop over all elements
  do {
    func(iterator, order, userData);
    // determine the next element according to order
    if (order == ASC)
      iterator = iterator->elementNext;
    else
      iterator = iterator->elementPrev;
  } while (iterator); // while (iterator != NULL)
}

// we print the name of the string name, the address of prev, current and next
// element
void print_element(const element_t *e, loop_order_t order, void *userData) {
  printf(
      "%10s: %15p --> %15p --> %15p\n", e->id, e->elementPrev, e,
      e->elementNext); // symbol/format string "%10s" used for printing a string
                       // max length = 10; %p format string for a pointer
}

void printElementIndex(const element_t *e, loop_order_t order, void *userData)
{
  size_t *integer = (size_t *)userData; // converting userData to a *int
  printf("list[%lu]: %s\n", *integer, e->id);
  if(order == ASC)
    (*integer)++;
  else
    (*integer)--;
}

//   _____ _____ ____ _____   __  __       _       
//  |_   _| ____/ ___|_   _| |  \/  | __ _(_)_ __  
//    | | |  _| \___ \ | |   | |\/| |/ _` | | '_ \
//    | | | |___ ___) || |   | |  | | (_| | | | | |
//    |_| |_____|____/ |_|   |_|  |_|\__,_|_|_| |_|
                                           
#define LIST_LENGTH 4
#define MAX_ELEMENT_LENGTH 10
int main() {
  char id[LIST_LENGTH][MAX_ELEMENT_LENGTH] = {"one", "two", "three",
                    "four"}; // array of LIST_LENGTH ID of max MAX_ELEMENT_LENGTH chars
  /*register*/ size_t counter; // to have access to this variable in a faster way since it is called many times

  // list creation creating a first new element
  list_t *list = list_new("zero");

  // populate the list with the other elements
  for (counter = 0; counter < LIST_LENGTH; counter++) {
    // create an element and append to the list
    element_t *e = elementNew(id[counter]);
    
    list_append_element(list, e);
    // or, ------------ alternatively-----------------:
    // list_append(list, id[counter]);
  }

  // providing a description for all the elements in the list: loop over the
  // list
  list_loop(list, print_element, ASC, NULL);

  list_insert(list, "two.five", "two");

  list_loop(list, print_element, DESC, NULL);

  counter = 0;
  list_loop(list, printElementIndex, ASC, &counter);

  counter = list->length - 1;
  list_loop(list, printElementIndex, DESC, &counter);

  list_free(list);

  return 0;
}
#undef LIST_LENGTH
#undef MAX_ELEMENT_LENGTH