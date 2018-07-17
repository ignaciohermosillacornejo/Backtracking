#include "stack.h"
#include "cell.h"
#include <stdlib.h>
#include <stdio.h>
/* Stack class created based on the solution to "actividad 01" from this course
 * and heavily modified
 */

/* Helper function to create a node */
Node *node_init(Cell *value)
{
  // Create our node
  Node *node = malloc(sizeof(Node));

  // Set its values and the next node
  node->value = value;
  node->next = NULL;

  // Return the node
  return node;
}

/* Create an empty stack and return a pointer to it */
Stack *stack_init()
{
  // Create our stack pointer
  Stack *stack = malloc(sizeof(Stack));

  // basic attributes
  stack->count = 0;
  stack->first = NULL;

  // return our stack
  return stack;
}

/* Insert an element into our stack*/
void stack_push(Stack *stack, Cell *element)
{
  // Create the node
  Node *node = node_init(element);

  // If the stack is empty
  if (!stack->count)
  {
    // We set the first element to the node
    stack->first = node;
  }
  else
  {
    // We point the new node to the first
    node->next = stack->first;
    // we make the first node the new node
    stack->first = node;
  }
  // Increase the count
  stack->count++;
}

/* eliminates the first node in the stack and returns its value
 * if the list has an element, else we return a NULL pointer
 */
Cell *stack_pop(Stack *stack)
{
  // create the pointer to return
  Cell *value = NULL;
  // check to see if the list is empty
  if (stack->count)
  {
    // set the value of the pointer to return
    value = stack->first->value;
    // we get the node to remove
    Node *first = stack->first;
    // we remove the node from the stack
    stack->first = first->next;
    // free the memory of the removed node
    free(first);
    // decrease the size by one
    stack->count--;
  }
  // return the value of the node
  return value;
}

/* Remove the given cell from the stack, returns NULL if the stack is empty
 * or the cell wasn't found
 */
Cell *stack_remove(Stack *stack, Cell * cell)
{
  Cell *value = NULL;
  if (stack->count) // we check the stack is not empty
  {
    Node * current = stack -> first;
    Node * previous = NULL;
    // in case the key we are looking is on the first node
    if (current -> value == cell)
    {
      return stack_pop(stack);
    }
    // we iterate over the stack looking for the key
    while (current && current -> value != cell)
    {
      previous = current;
      current = current -> next;
    }

    // the key you are looking for is not here, return NULL pointer
    if (!current)
    {
      return value;
    }
    previous -> next = current -> next;
    value = current -> value;
    free(current);
    stack -> count--;
  }
  return value;

}

/* We free all the resources of the stack */
void stack_destroy(Stack *stack)
{
  // Remove all the nodes until the list is empty
  while (stack_pop(stack))
  {
    //printf("WARNING: Destroyed stack has elements left on it\n");
  }
  // Free the stack
  free(stack);
}
