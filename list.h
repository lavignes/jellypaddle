/**
 * A simple doubly-linked list
 * @author Scott LaVigne
 */
#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct List {

  size_t length;
  struct ListNode* head;
  struct ListNode* tail;

} List;

/**
 * Create a new list.
 * @return a new list
 */
List* list_new();

/**
 * Free a list.
 * @param list the list to free
 */
void list_free(List* list);

/**
 * Add element to end of list.
 * @param list a list
 * @param data element to append
 */
void list_push_back(List* list, void* data);

/**
 * Return and remove element from end of list.
 * @param list a list
 */
void* list_pop_back(List* list);

/**
 * Add element to front of list.
 * @param list a list
 * @param data element to prepend
 */
void list_push_front(List* list, void* data);

/**
 * Return and remove element from front of list.
 * @param list a list
 */
void* list_pop_front(List* list);

/**
 * Return element on end of list.
 * @param list a list
 */
void* list_peek_back(List* list);

/**
 * Return element on front of list.
 * @param list a list
 */
void* list_peek_front(List* list);

/**
 * Apply a function to every element in the list.
 * @param list a list
 * @param fn   a funtion to apply. It's first argument
 *             is the element, and the second argument
 *             is extra data. If it ever returns true,
 *             the traversal will end.
 * @param data extra data to pass to the funtion
 */
void list_traverse(List* list, bool(*fn)(void*, void*), void* data);

#endif /* LIST_H */
