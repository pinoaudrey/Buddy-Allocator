/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "libpriqueue.h"


// void node_init(node *n, void *ptr)
// {
//   node *newNode = (node *) malloc(sizeof(node));
//   *n = *newNode;
//   n->data = ptr;
//   n->next = NULL;
//   n->prev = NULL;
// }

// void node_return(node *n)
// {
//   return n->data;
// }

// void node_link(node *source, node *target)
// {
//   source->next = target;
//   source->prev = target->prev;
//   target->prev->next = source;
//   target->prev = source;
// }

/**
  Initializes the priqueue_t data structure.
  
  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
  // priqueue_t *newQue = (priqueue_t *) malloc(sizeof(priqueue_t));
  // *q = *newQue;
  q->size = 0;
  q->comparer = comparer;
  // node new_head;
  // node_init(&new_head, 88);
  // q->head = &new_head;
  q->head = (node *) malloc(sizeof(node));
  q->head->data = 0;
  q->head->next = NULL;
  q->head->prev = NULL;

  // node new_tail;
  // node_init(&new_tail, 88);
  // q->tail = &new_tail;
  q->tail = (node *) malloc(sizeof(node));
  q->tail->data = 0;
  q->tail->next = NULL;
  q->tail->prev = NULL;

  // node_link(q->head, q->tail);
  q->tail->prev = q->head;
  q->head->next = q->tail;
}


/**
  Insert the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
  node *new = (node *) malloc(sizeof(node));
  new->data = ptr;
  // node_init(&new, ptr);

  node *curr = q->head->next;
  int position = 0;
  bool added = false;

  for (int i = 0; i < q->size; i++) {
    int value = q->comparer(ptr, curr->data);

    if (value >= 0) {
      //IDK if this is right, but this code should be for the case
      //where we want to progress to the next node without inserting here.
      position++;
      curr = curr->next;
    } else {
      //This code should be for the case where you do want to insert the element at that position.
      new->next = curr;
      new->prev = curr->prev;
      curr->prev->next = new;
      curr->prev = new;
      added = true;
      break;
    }
  }

  if (!added)
  {
    //The case where we got to the end of the iteration and were not able to find a place to insert.
    node *curr = q->tail;
    new->next = curr;
    new->prev = curr->prev;
    curr->prev->next = new;
    curr->prev = new;
  }

  q->size++;
	return position;
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
  if (q->size == 0)
	  return NULL;
 
  node *first = q->head->next;
  return first->data;
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
  if (q->size == 0)
	  return NULL;

  node *old_head = q->head->next;
  node *new_head = old_head->next;
  q->head->next = new_head;
  new_head->prev = q->head;
  q->size--;
  
  void * retVal = old_head->data;
  free(old_head);

  return retVal;
}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
  if (index >= q->size) 
    return NULL;

  node *temp = q->head->next;
  for (int i = 0; i < index; ++i) {
    temp = temp->next;
  }
  return temp->data;
}


/**
  Removes all instances of ptr from the queue. 
  
  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
  int count = 0;
  node *curr = q->head->next;
  int iterations = q->size;
  for (int i = 0; i < iterations; ++i)
  {
    node *next_node = curr->next; //get the next node for iteration.

    if (curr->data == ptr) 
    {
      node *prev_node = curr->prev;
      // node *next_node = curr->next;

      prev_node->next = next_node;
      next_node->prev = prev_node;

      count++;
      q->size--;
      
      free(curr);
    }

    curr = next_node;
  }
	return count;
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
	if (index >= q->size)
    return NULL;

  node *curr = q->head->next;
  for (int i = 0; i != index; ++i) {
    curr = curr->next;
  }

  node *prev_node = curr->prev;
  node *next_node = curr->next;
  prev_node->next = next_node;
  next_node->prev = prev_node;
  
  q->size--;

  void *retVal = curr->data;

  free(curr);

  return retVal;
}


/**
  Return the number of elements in the queue.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return q->size;
}


/**
  Destroys and frees all the memory associated with q.
  
  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{
  node *curr = q->head->next;

  while(curr != q->tail)
  {
    node * next = curr->next;
    free(curr);
    curr = next;
  }

  free(q->head);
  free(q->tail);

  //free(q);
}
