/** @file libpriqueue.h
 */

#ifndef LIBPRIQUEUE_H_
#define LIBPRIQUEUE_H_

/*
  Node Data Structure
*/
typedef struct _node
{
  void *data;
  struct _node *next;
  struct _node *prev;
} node;

// void node_init(node *n, void *ptr);
// void node_return(node *n);
// void node_link(node *source, node *target);

/**
  Priqueue Data Structure
*/
typedef struct _priqueue_t
{
  int size;
  int(*comparer)(const void *, const void *);
  node *head;
  node *tail;
} priqueue_t;


void   priqueue_init     (priqueue_t *q, int(*comparer)(const void *, const void *));

int    priqueue_offer    (priqueue_t *q, void *ptr);
void * priqueue_peek     (priqueue_t *q);
void * priqueue_poll     (priqueue_t *q);
void * priqueue_at       (priqueue_t *q, int index);
int    priqueue_remove   (priqueue_t *q, void *ptr);
void * priqueue_remove_at(priqueue_t *q, int index);
int    priqueue_size     (priqueue_t *q);

void   priqueue_destroy  (priqueue_t *q);


#endif /* LIBPQUEUE_H_ */
