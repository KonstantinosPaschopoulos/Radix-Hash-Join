#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "functions.h"
#include "buffer.h"
#include "types.h"

void userInput(int32_t **A, int32_t **B, int sizeAx, int sizeAy, int sizeBx, int sizeBy, int posA, int posB, relation *relR, relation *relS)
{
  int32_t i;
  tuple tmp;

  relR->tuples = (tuple *)malloc(sizeAx * sizeof(tuple));
  if (relR->tuples == NULL)
  {
    perror("Unable to allocate space");
    exit(-1);
  }
  relS->tuples = (tuple *)malloc(sizeBx * sizeof(tuple));
  if (relS->tuples == NULL)
  {
    perror("Unable to allocate space");
    exit(-1);
  }

  relR->num_tuples = 0;
  relS->num_tuples = 0;

  for (i = 0; i < sizeAx; i++)
  {
    tmp.payload = A[i][posA];
    tmp.key = i; //rowId

    relR->tuples[i] = tmp;

    relR->num_tuples++;
  }

  for (i = 0; i < sizeBx; i++)
  {
    tmp.payload = B[i][posB];
    tmp.key = i; //rowId

    relS->tuples[i] = tmp;

    relS->num_tuples++;
  }
}

void histogram(relation *rel, relation *hist)
{
  int32_t i, masked;

  hist->tuples = (tuple *)malloc(256 * sizeof(tuple));
  if (hist->tuples == NULL)
  {
    perror("Unable to allocate space");
    exit(-1);
  }
  hist->num_tuples = 256;

  for (i = 0; i < hist->num_tuples; i++)
  {
    hist->tuples[i].key = 0;
    hist->tuples[i].payload = i;  //mask
  }

  for (i = 0; i < rel->num_tuples; i++)
  {
    masked = hashFunc(rel->tuples[i].payload);
    hist->tuples[masked].key++; //sum
  }
}

void Psum(relation *hist, relation *psum)
{
  int32_t i;

  psum->tuples = (tuple*)malloc((hist->num_tuples) * sizeof(tuple));
  if (psum->tuples == NULL)
  {
    perror("Unable to allocate space");
    exit(-1);
  }
  psum->num_tuples = hist->num_tuples;

  psum->tuples[0].key = 0;  //address
  psum->tuples[0].payload = hist->tuples[0].payload; //mask

  for (i = 1; i < psum->num_tuples; i++)
  {
    psum->tuples[i].key = psum->tuples[i - 1].key + hist->tuples[i - 1].key;
    psum->tuples[i].payload = hist->tuples[i].payload;
  }
}

void reorderedR(relation *psum, relation *newR, relation *relR)
{
  int32_t i, x;

  newR->tuples = (tuple*)malloc(relR->num_tuples * sizeof(tuple));
  if (newR->tuples == NULL)
  {
    perror("Unable to allocate space");
    exit(-1);
  }
  newR->num_tuples = relR->num_tuples;

  for (i = 0; i < relR->num_tuples; i++)
  {
    x = hashFunc(relR->tuples[i].payload);

    newR->tuples[psum->tuples[x].key].key = relR->tuples[i].key;
    newR->tuples[psum->tuples[x].key].payload = relR->tuples[i].payload;

    (psum->tuples[x].key)++;
  }
}

int32_t hashFunc(int32_t payload)
{
   return payload & 255; //11111111(binary) == 255(decimal)
}

void indexFunc(relation *bucketN, int32_t *chain, int32_t *bucket, int32_t hash_number)
{
  int32_t i, x, next;

  for (i = 0; i < hash_number; i++)
  {
    bucket[i] = -1;
  }

  for (i = 0; i < bucketN->num_tuples; i++)
  {
    chain[i] = -1;
  }

  for (i = (bucketN->num_tuples - 1); i >= 0; i--)
  {
    x = bucketN->tuples[i].payload % hash_number; //hash function

    //If the slot is empty fill it
    if (bucket[x] == -1)
    {
      bucket[x] = i;
    }
    else
    {
      if (chain[bucket[x]] == -1)
      {
        chain[bucket[x]] = i;
      }
      else
      {
        next = chain[bucket[x]];
        while (chain[next] != -1)
        {
          next = chain[next];
        }

        chain[next] = i;
      }
    }
  }
}

int32_t hashFunc2(int32_t payload, int32_t num)
{
   return payload % num;
}

result* RadixHashJoin(relation *relR, relation *relS)
{
  int32_t i, j, hash_number, x, next;
  int32_t *chain, *bucket;
  relation histStructR, histStructS, psumStructR, psumStructS, reorderedStructR, reorderedStructS,
          smallB, copypsumStructR, copypsumStructS;
  relation *histR = &histStructR, *histS = &histStructS, *psumR = &psumStructR,
          *psumS = &psumStructS, *reR = &reorderedStructR, *reS = &reorderedStructS, *smallBucket = &smallB,
          *copypsumR = &copypsumStructR, *copypsumS = &copypsumStructS;

  //In the first phase we create a histogram and an additive histogram
  //and then we reorder the initial relations

  //We create the histograms for both of the relations
  histogram(relR, histR);
  histogram(relS, histS);

  //Now we can create the additive histograms
  Psum(histR, psumR);
  Psum(histS, psumS);
  //since the psums are going to change when we create the
  //reordered array, we keep a copy of them
  Psum(histR, copypsumR);
  Psum(histS, copypsumS);
  //Finally we have all the information we need in order to re-order the relations
  reorderedR(psumR, reR, relR);
  free(relR->tuples);
  free(psumR->tuples);
  reorderedR(psumS, reS, relS);
  free(relS->tuples);
  free(psumS->tuples);

  //Creating the buffer
  resultPtr head = NULL, curr = NULL;
  head = bufferInit();
  curr = head;

  //In the second phase we have to create an index for each bucket
  //and in the third phase we compare the values of the buckets

  //comparing every bucket from S and R
  for (i = 0; i < histS->num_tuples; i++)
  {
    //The buckets need to have elements
    if ((histR->tuples[i].key > 0) && (histS->tuples[i].key > 0))
    {
      //finding the smallest bucket
      if ((histR->tuples[i].key) > (histS->tuples[i].key))
      {
        //relS has the smallest bucket
        tuple* small = (tuple*)malloc((histS->tuples[i].key) * sizeof(tuple));
        smallBucket->tuples = small;
        smallBucket->num_tuples = histS->tuples[i].key;

        //copying the smallest bucket to an array
        for(j = 0; j < histS->tuples[i].key; j++)
        {
          smallBucket->tuples[j].payload = reS->tuples[j + copypsumS->tuples[i].key].payload;
          smallBucket->tuples[j].key = reS->tuples[j + copypsumS->tuples[i].key].key;
        }

        //size of the smallest bucket
        chain = (int32_t*)malloc((smallBucket->num_tuples) * sizeof(int32_t));
        //depends on the size of the hash function
        hash_number = (smallBucket->num_tuples * 2) + 1;
        bucket = (int32_t*)malloc(hash_number * sizeof(int32_t));

        //Creating the index for the smallest bucket
        indexFunc(smallBucket, chain, bucket, hash_number);

        for (j = copypsumR->tuples[i].key; j < copypsumR->tuples[i].key + histR->tuples[i].key; j++)
        {
          x = hashFunc2(reR->tuples[j].payload, hash_number);

          if (bucket[x] != -1)
          {
            insertTuple(curr, reR->tuples[j].key, smallBucket->tuples[bucket[x]].key);
            next = bucket[x];
            while (chain[next] != -1)
            {
              insertTuple(curr, reR->tuples[j].key, smallBucket->tuples[chain[next]].key);
              next = chain[next];
            }
          }
        }

        free(bucket);
        free(chain);
        free(small);
      }
      else
      {
        //relR has the smallest bucket
        tuple* small = (tuple*)malloc((histR->tuples[i].key) * sizeof(tuple));
        smallBucket->tuples = small;
        smallBucket->num_tuples = histR->tuples[i].key;

        //copying the smallest bucket to an array
        for(j = 0; j < histR->tuples[i].key; j++)
        {
          smallBucket->tuples[j].payload = reR->tuples[j + copypsumR->tuples[i].key].payload;
          smallBucket->tuples[j].key = reR->tuples[j + copypsumR->tuples[i].key].key;
        }

        //size of the smallest bucket
        chain = (int32_t*)malloc((smallBucket->num_tuples) * sizeof(int32_t));
        //depends on the size of the hash function
        hash_number = (smallBucket->num_tuples * 2) + 1;
        bucket = (int32_t*)malloc(hash_number * sizeof(int32_t));

        //Creating the index for the smallest bucket
        indexFunc(smallBucket, chain, bucket, hash_number);

        for (j = copypsumS->tuples[i].key; j < copypsumS->tuples[i].key + histS->tuples[i].key; j++)
        {
          x = hashFunc2(reS->tuples[j].payload, hash_number);

          if (bucket[x] != -1)
          {
            insertTuple(curr, smallBucket->tuples[bucket[x]].key, reS->tuples[j].key);
            next = bucket[x];
            while (chain[next] != -1)
            {
              insertTuple(curr, smallBucket->tuples[chain[next]].key, reS->tuples[j].key);
              next = chain[next];
            }
          }
        }

        free(bucket);
        free(chain);
        free(small);
      }
    }
  }

  return head;
}
