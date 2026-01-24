#include "vector.h"


FoVector* createVec(int typeLength)
{
    void* data_ = malloc(typeLength * INIT_VEC_SIZE);
    FoVector* new_vec = (FoVector*)malloc(sizeof(FoVector));
    new_vec->data = data_;
    new_vec->typeLength = typeLength;
    new_vec->size = 0;
    new_vec->capacity = INIT_VEC_SIZE;
    return new_vec;
}
void freeVec(FoVector* vec)
{
    free(vec->data);
    free(vec);
}

bool vecReachLimit(const FoVector* vec)
{
    return ((vec->size >= vec->capacity) ? true : false);
}
void vecDoubleCapacity(FoVector* vec)
{
    vec->data = realloc(vec->data, vec->typeLength * vec->capacity * 2);
    vec->capacity *= 2;
}

void vecPushBack(FoVector* vec, const void* data)
{
    if (vecReachLimit(vec))
    {
        vecDoubleCapacity(vec);
    }
    char* handleP1 = (char*)vec->data;
    //char* handleP2 = (char*)data;
    handleP1 += vec->size * vec->typeLength;
    memcpy(handleP1, data, vec->typeLength);
    vec->size++;
}
void vecPopBack(FoVector* vec)
{
    if (vec != NULL && vec->size != 0)
    {
        vec->size--;
    }
}

void* vecAt(const FoVector* vec, int32_t index)
{
    if (vec == NULL || index < 0 || index >= vec->size)
    {
        return NULL;
    }
    char* resultP = vec->data;
    resultP += index * vec->typeLength;
    return (void*)resultP;
}
void vecAtAssign(FoVector* vec, int32_t index, void* target)
{
    if (vec == NULL || index < 0 || index >= vec->size)
    {
        return;
    }
    char* resultP = vec->data;
    resultP += index * vec->typeLength;
    memcpy(resultP, target, vec->typeLength);
}

void vecClear(FoVector* vec)
{
    vec->size = 0;
}

void vecReserve(FoVector* vec, int32_t newCapacity)
{
    if (newCapacity <= vec->capacity)
    {
        return;
    }
    vec->data = realloc(vec->data, newCapacity * vec->typeLength);
    vec->capacity = newCapacity;
}
void vecResize(FoVector* vec, int32_t newSize, void* fillValue)
{
    if (newSize > vec->capacity)
    {
        vecReserve(vec, newSize);
    }
    if (newSize > vec->size)
    {
        char* handle = (char*)vec->data + vec->size * vec->typeLength;
        for (int i = vec->size; i < newSize; i++)
        {
            memcpy(handle, fillValue, vec->typeLength);
            handle += vec->typeLength;
        }
    }
    vec->size = newSize;
}

void vecCopy(FoVector* dest, const FoVector* source)
{
    if (dest == NULL || source == NULL)
    {
        return;
    }
    vecReserve(dest, source->capacity);
    memcpy(dest->data, source->data, source->size * source->typeLength);
    dest->size = source->size;
}