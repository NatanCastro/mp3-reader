#ifndef DLINKED_LIST_H_
#define DLINKED_LIST_H_

typedef struct dll_node {
  struct dll_node *prev;
  void *data;
  struct dll_node *next;
} dll_node_t;

typedef struct {
  dll_node_t *head;
  dll_node_t *tail;
} dll_t;

void dll_append(dll_t *, void *);
void dll_prepend(dll_t *, void *);

#endif // !DLINKED_LIST_H_
