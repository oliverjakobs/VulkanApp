#include "array.h"


void* _obeliskArrayGrow(void* arr, size_t stride, size_t increment, size_t minCap) {
    // compute the minimum capacity needed
    size_t newCap = obeliskArrayLen(arr) + increment;
    if (minCap > newCap) newCap = minCap;

    if (newCap <= obeliskArrayCap(arr)) return arr;

    // increase needed capacity to guarantee O(1) amortized
    if (newCap < 2 * obeliskArrayCap(arr))  newCap = 2 * obeliskArrayCap(arr);
    else if (newCap < 4)                    newCap = 4;

    size_t hdrSize = OBELISK_ARRAY_HDR_SIZE * sizeof(size_t);
    size_t newSize = stride * newCap;

    if (newSize <= 0) return arr;

    size_t* hdr = realloc((arr) ? _obeliskArrayHdr(arr) : NULL, newSize + hdrSize);

    if (!hdr) return NULL; /* out of memory */

    if (arr == NULL) hdr[1] = 0;

    hdr[0] = newCap;

    return hdr + OBELISK_ARRAY_HDR_SIZE;
}