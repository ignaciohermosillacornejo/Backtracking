#pragma once
#include "board.h"

/* structure of the node that we use in our stack */
struct node
{
  /* pointer to our next node */
  struct node *next;
  /* value of the node */
  Cell *value;
};

typedef struct node Node;

/** Structure of the stack */
struct stack
{
  /* Pointer to the first node */
  Node *first;
  /* Amount of nodes */
  int count;
};

/* Structure of the stack */
typedef struct stack Stack;

/* Crea an empty stack and return the pointer */
Stack *stack_init();

/* Push an element into the stack */
void stack_push(Stack *stack, Cell *element);

/* Pop an element from the stack and return it */
Cell *stack_pop(Stack *stack);

/* Free all the memory associated of the stack */
void stack_destroy(Stack *stack);
