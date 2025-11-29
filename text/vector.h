#ifndef __FOVECTOR_T__
#define __FOVECTOR_T__

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define INIT_VEC_SIZE 4

typedef struct FoVector
{
    void* data;
    int32_t typeLength;
    int32_t size;
    int32_t capacity;
}FoVector;

FoVector* createVec(int typeLength);
void freeVec(FoVector* vec);

bool vecReachLimit(const FoVector* vec);
void vecDoubleCapacity(FoVector* vec);

void vecPushBack(FoVector* vec, const void* data);
void vecPopBack(FoVector* vec);

void* vecAt(const FoVector* vec, int32_t index);
void vecAtAssign(FoVector* vec, int32_t index, void* target);

void vecClear(FoVector* vec);

void vecReserve(FoVector* vec, int32_t new_capacity);
void vecResize(FoVector* vec, int32_t newSize, void* fillValue);

void vecCopy(FoVector* dest, const FoVector* source);

static inline int32_t vecGetSize(const FoVector* vec)
{
    return vec->size;
}
static inline int32_t vecGetCapacity(const FoVector* vec)
{
    return vec->capacity;
}
#endif