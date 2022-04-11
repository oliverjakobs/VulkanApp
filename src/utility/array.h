#ifndef OBELISK_ARRAY_H
#define OBELISK_ARRAY_H

#include "../common.h"

#include <string.h>

#define OBELISK_ARRAY_HDR_SIZE  2

#define _obeliskArrayHdr(a) ((size_t*)(a) - OBELISK_ARRAY_HDR_SIZE)
#define _obeliskArrayCap(a) (_obeliskArrayHdr(a)[0])
#define _obeliskArrayLen(a) (_obeliskArrayHdr(a)[1])

#define obeliskArrayCap(a)  ((a) ? _obeliskArrayCap(a) : 0)
#define obeliskArrayLen(a)  ((a) ? _obeliskArrayLen(a) : 0)

#define obeliskArrayMakeSpace(a,n)  ((a) = _obeliskArrayReserve((a),  sizeof(*(a)), obeliskArrayLen(a) + (n)))
#define obeliskArrayReserve(a,n)    ((a) = _obeliskArrayReserve((a),  sizeof(*(a)), n))
#define obeliskArrayPack(a)         ((a) = _obeliskArrayResize((a),  sizeof(*(a)), obeliskArrayLen(a)))

#define obeliskArrayFree(a)         (((a) ? free(_obeliskArrayHdr(a)) : 0), (a) = NULL)

#define obeliskArrayPush(a,v)   (obeliskArrayMakeSpace(a,1), (a)[_obeliskArrayLen(a)++] = (v))
#define obeliskArrayPushN(a,n)  (obeliskArrayMakeSpace(a,n), (n) ? (_obeliskArrayLen(a)+=(n), &(a)[_obeliskArrayLen(a)-(n)]) : (a))
#define obeliskArrayPop(a)      ((a)[--_obeliskArrayLen(a)])

#define obeliskArrayRemoveN(a,i,n)  (_obeliskRemoveN((a), sizeof(*(a)), (i), (n)))
#define obeliskArrayRemove(a,i)     (obeliskArrayRemoveN(a,i,1))

#define obeliskArrayInsertN(a,i,n)  (obeliskArrayMakeSpace(a, n), _obeliskInsertN((a), sizeof(*(a)), (i), (n)))
#define obeliskArrayInsert(a,i,v)   (obeliskArrayInsertN(a,i,1), (a)[i]=(v))

// #define obeliskArrayLast(a)       ((a)[_obeliskArrayLen(a) - 1])
#define obeliskArrayLast(b)    ((b) + (obeliskArrayLen(b) - 1))

void* _obeliskArrayResize(void* arr, size_t stride, size_t newCap);
void* _obeliskArrayReserve(void* arr, size_t stride, size_t newCap);

void* _obeliskInsertN(void* arr, size_t stride, size_t i, size_t n);
void* _obeliskRemoveN(void* arr, size_t stride, size_t i, size_t n);

#endif // !OBELISK_ARRAY_H
