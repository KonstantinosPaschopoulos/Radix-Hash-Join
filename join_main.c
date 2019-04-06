#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "functions.h"
#include "buffer.h"
#include "types.h"

int main(void)
{
  int32_t i, j;
  const int32_t c = 5, r = 100;
  relation structR, structS;
  relation *relR = &structR, *relS = &structS;
  int32_t **A = (int32_t **)malloc(r * sizeof(int32_t *));
  int32_t **B = (int32_t **)malloc(r * sizeof(int32_t *));


  for (i = 0; i < r; i++)
  {
    A[i] = (int32_t *)malloc(c * sizeof(int32_t));
  }
  for (i = 0; i < r; i++)
  {
    B[i] = (int32_t *)malloc(c * sizeof(int32_t));
  }

  for (i = 0; i < r; i++)
  {
      for (j = 0; j < c; j++)
      {
        A[i][j] = i;
        B[i][j] = i;
      }
  }

  //The first thing we do is take the user input and keep only
  //the columns that are needed to perform the join
  userInput(A, B, r, c, r, c, 1, 1, relR, relS);

  //After the preprocessing is done we can call the join function
  resultPtr buff = RadixHashJoin(relR, relS);

  printBuffer(buff);

  return 0;
}
