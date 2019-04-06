#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>

#define BUFFER_ROWS ((1024 * 1024) / 8)

typedef struct result *resultPtr;

typedef struct tuple {
  int32_t key;
  int32_t payload;
} tuple;

typedef struct relation {
  tuple *tuples;
  uint32_t num_tuples;
} relation;

typedef struct result {
  int32_t buffer[BUFFER_ROWS][2];
  struct result *next;
  uint32_t num_tuples;
} result;

#endif
