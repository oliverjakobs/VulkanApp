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


#define obeliskArrayMakeSpace(a,n)  ((!(a) || _obeliskArrayLen(a) + (n) > _obeliskArrayCap(a)) ? (stbds_arrgrow(a,n,0),0) : 0)
#define stbds_arrgrow(a,b,c)   ((a) = _obeliskArrayGrow((a), sizeof(*(a)), (b), (c)))


#define stbds_arrreserve(a,n)  (stbds_arrgrow(a,0,n))
#define stbds_arrfree(a)       (((a) ? free(_obeliskArrayHdr(a)) : 0), (a) = NULL)

#define obeliskArrayPush(a,v)   (obeliskArrayMakeSpace(a,1), (a)[_obeliskArrayLen(a)++] = (v))
#define obeliskArrayPushN(a,n)  (obeliskArrayMakeSpace(a,n), (n) ? (_obeliskArrayLen(a) += (n), &(a)[_obeliskArrayLen(a)-(n)]) : (a))
#define obeliskArrayPop(a)      (_obeliskArrayLen(a)--, (a)[_obeliskArrayLen(a)])


#define obeliskArrayRestSize(a,i,n) (sizeof(*(a)) * (_obeliskArrayLen(a)-(n)-(i)))

#define stbds_arrdeln(a,i,n)   (memmove(&(a)[i], &(a)[(i)+(n)], obeliskArrayRestSize(a,i,n)), _obeliskArrayLen(a) -= (n))
#define stbds_arrdel(a,i)      (stbds_arrdeln(a,i,1))

#define stbds_arrinsn(a,i,n)   (((void)(obeliskArrayPushN(a, n))), memmove(&(a)[(i)+(n)], &(a)[i], obeliskArrayRestSize(a,i,n)))
#define stbds_arrins(a,i,v)    (stbds_arrinsn(a,i,1), (a)[i]=(v))




// #define stbds_arrlast(a)       ((a)[_obeliskArrayLen(a) - 1])
#define obeliskArrayLast(b)    ((b) + (obeliskArrayLen(b) - 1))

void* _obeliskArrayGrow(void* arr, size_t stride, size_t increment, size_t minCap);

void* _obeliskArrayResize(void* arr, size_t stride, size_t newCap);
void* _obeliskArrayReserve(void* arr, size_t stride, size_t newCap);

#endif // !OBELISK_ARRAY_H
