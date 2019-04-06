#include "types.h"

void userInput(int32_t **, int32_t **, int, int, int, int, int, int, relation *, relation *);
result* RadixHashJoin(relation*, relation*);
void histogram(relation *, relation *);
void Psum(relation *, relation *);
void reorderedR(relation *, relation *, relation *);
int32_t hashFunc(int32_t);
void indexFunc(relation *, int32_t *, int32_t *, int32_t);
int32_t hashFunc2(int32_t, int32_t);
