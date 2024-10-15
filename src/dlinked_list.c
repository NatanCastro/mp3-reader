#include "dlinked_list.h"
#include <stdlib.h>
#include <string.h>

void dll_append(dll_t *list, void *data) {
  dll_node_t *node = (dll_node_t *)malloc(sizeof(dll_node_t));
  if (!node) {
    return;
  }

  node->prev = NULL;
  node->data = data;
  node->next = NULL;

  if (list->head == NULL && list->tail == NULL) {
    list->head = node;
    list->tail = node;
    return;
  }

  list->tail->next = node;
  node->prev = list->tail;
  list->tail = node;
}

void dll_prepend(dll_t *list, void *data) {
  dll_node_t *node = (dll_node_t *)malloc(sizeof(dll_node_t));
  if (!node) {
    return;
  }

  node->prev = NULL;
  node->data = data;
  node->next = NULL;

  if (list->head == NULL && list->tail == NULL) {
    list->head = node;
    list->tail = node;
    return;
  }

  list->head->prev = node;
  node->next = list->head;
  list->head = node;
}
