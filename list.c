/**
 * A simple doubly-linked list: implementation
 * @author Scott LaVigne
 */
#include "list.h"

typedef struct ListNode {

  void* data;
  struct ListNode* next;
  struct ListNode* prev;

} ListNode;

List* list_new() {
  List* list = malloc(sizeof(List));
  list->length = 0;
  list->head = list->tail = NULL;
  return list;
}

void list_free(List* list) {
  ListNode* node = list->head;
  ListNode* temp;
  while (node != NULL) {
    temp = node->next;
    free(node);
    node = temp;
  }
  free(list);
}

void list_push_back(List* list, void* data) {
  list->length++;
  ListNode* node = malloc(sizeof(ListNode));
  node->data = data;
  node->next = NULL;
  if (list->tail != NULL) {
    node->prev = list->tail;
    list->tail->next = node;
  }
  if(list->head == NULL)
    list->head = node;
  list->tail = node;
}

void* list_pop_back(List* list) {
  void* data = NULL;
  ListNode* node = list->tail;
  if (node != NULL) {
    list->length--;
    if (list->tail->prev != NULL) {
      list->tail = node->prev;
      list->tail->next = NULL;
    } else {
      list->head = list->tail = NULL;
    }
    data = node->data;
    free(node);
  }
  return data;
}

void list_push_front(List* list, void* data) {
  list->length++;
  ListNode* node = malloc(sizeof(ListNode));
  node->data = data;
  node->prev = NULL;
  if (list->head != NULL) {
    node->next = list->head;
    list->head->prev = node;
  }
  list->head = node;
  if (list->tail == NULL)
    list->tail = node;
}

void* list_pop_front(List* list) {
  ListNode* node = list->head;
  void* data = NULL;
  if (list->head != NULL) {
    list->length--;
    if (list->head->next != NULL) {
      list->head = list->head->next;
      list->head->prev = NULL;
    } else {
      list->head= NULL;
      list->tail = NULL;
    }
    data = node->data;
    free(node);
  }
  return data;
}

void* list_peek_back(List* list) {
  return list->tail;
}

void* list_peek_front(List* list) {
  return list->head;
}

void list_traverse(List* list, bool(*fn)(void*, void*), void* data) {
  ListNode* node = list->head;
  while (node != NULL) {
    if (fn(node->data, data))
      break;
    node = node->next;
  }
}