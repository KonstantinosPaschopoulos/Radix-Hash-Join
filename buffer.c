#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "buffer.h"
#include "types.h"

resultPtr bufferInit(){
  resultPtr head;
  head = (result *)malloc(sizeof(result));
  if (head == NULL)
  {
    perror("Unable to allocate space");
    exit(-1);
  }
  head->num_tuples = 0;
  head->next = NULL;

  return head;
}

void insertNewBuffer(resultPtr currentLast)
{
  resultPtr newLast;

  newLast = (result *)malloc(sizeof(result));
  if (newLast == NULL)
  {
    perror("Unable to allocate space");
    exit(-1);
  }
  newLast->num_tuples = 0;
  newLast->next = NULL;

  currentLast->next = newLast;
}

void insertTuple(resultPtr head, int32_t A, int32_t B)
{
  result *curr = head;
  resultPtr prv = curr;
  int flag = 0;

  while (curr != NULL)
  {
    if (curr->num_tuples < BUFFER_ROWS)
    {
      curr->buffer[curr->num_tuples][0] = A;
      curr->buffer[curr->num_tuples][1] = B;

      curr->num_tuples++;

      flag = 1;
    }

    prv = curr;
    curr = curr->next;
  }

  if (flag == 0)
  {
    insertNewBuffer(prv);
    curr = prv->next;

    curr->buffer[curr->num_tuples][0] = A;
    curr->buffer[curr->num_tuples][1] = B;

    curr->num_tuples++;
  }

}

void printBuffer(resultPtr head){
  result *curr = head;
  int32_t i;

  while (curr != NULL)
  {
    for (i = 0; i < (curr->num_tuples); i++)
    {
      printf("R rowId %d \t", curr->buffer[i][0]);
      printf("S rowId %d \n", curr->buffer[i][1]);
    }

    curr = curr->next;
  }
}
