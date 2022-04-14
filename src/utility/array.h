#ifndef OBELISK_ARRAY_H
#define OBELISK_ARRAY_H

#include "../common.h"

#define OBELISK_ARRAY_HDR_SIZE  2

/* header access */
#define _obeliskArrayHdr(a) ((size_t*)(a) - OBELISK_ARRAY_HDR_SIZE)
#define _obeliskArrayCap(a) (_obeliskArrayHdr(a)[0])
#define _obeliskArrayLen(a) (_obeliskArrayHdr(a)[1])

/* returns the total number of elements the array can contain */
#define obeliskArrayCap(a)  ((a) ? _obeliskArrayCap(a) : 0)
/* returns the number of elements in the array */
#define obeliskArrayLen(a)  ((a) ? _obeliskArrayLen(a) : 0)

/* makes space for n new elements */
#define obeliskArrayMakeSpace(a,n)  ((a) = _obeliskArrayReserve((a), sizeof(*(a)), obeliskArrayLen(a) + (n)))
/* reserves storage space for n elements, does not change the length of the array */
#define obeliskArrayReserve(a,n)    ((a) = _obeliskArrayReserve((a), sizeof(*(a)), n))
/* shrinks the arrays capacity to fit the length of the array */
#define obeliskArrayPack(a)         ((a) = _obeliskArrayResize((a), sizeof(*(a)), obeliskArrayLen(a)))
/* frees the array */
#define obeliskArrayFree(a)         (((a) ? free(_obeliskArrayHdr(a)) : 0), (a) = NULL)

/* appends the item v to the end of the array a */
#define obeliskArrayPush(a,v)  (obeliskArrayMakeSpace(a,1), (a)[_obeliskArrayLen(a)++] = (v))
/* appends n uninitialized items at the end of the array, returns a pointer to the first uninitialized item added */
#define obeliskArrayPushN(a,n) (obeliskArrayMakeSpace(a,n), (n) ? (_obeliskArrayLen(a)+=(n), &(a)[_obeliskArrayLen(a)-(n)]) : (a))
/* removes the last element of the array and returns it */
#define obeliskArrayPop(a)     ((a)[--_obeliskArrayLen(a)])

/* deletes n elements starting at index i, moving the rest of the array over */
#define obeliskArrayRemoveN(a,i,n)  (_obeliskRemoveN((a), sizeof(*(a)), (i), (n)))
/* deletes the element at index i, moving the rest of the array over */
#define obeliskArrayRemove(a,i)     (obeliskArrayRemoveN(a,i,1))

/* inserts n uninitialized items into the array a starting at a[i], moving the rest of the array over */
#define obeliskArrayInsertN(a,i,n)  (obeliskArrayMakeSpace(a, n), _obeliskInsertN((a), sizeof(*(a)), (i), (n)))
/* inserts the item v into the array a, at index i, moving the rest of the array over */
#define obeliskArrayInsert(a,i,v)   (obeliskArrayInsertN(a,i,1), (a)[i]=(v))

/* returns the last element of the array a */
#define obeliskArrayLast(a) ((a)[_obeliskArrayLen(a) - 1])

/* internal functions */
void* _obeliskArrayResize(void* arr, size_t stride, size_t newCap);
void* _obeliskArrayReserve(void* arr, size_t stride, size_t newCap);

void* _obeliskInsertN(void* arr, size_t stride, size_t i, size_t n);
void* _obeliskRemoveN(void* arr, size_t stride, size_t i, size_t n);

#endif // !OBELISK_ARRAY_H
