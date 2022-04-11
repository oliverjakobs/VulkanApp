#include "array.h"


void* _obeliskArrayResize(void* arr, size_t stride, size_t cap) {
    size_t hdrSize = OBELISK_ARRAY_HDR_SIZE * sizeof(size_t);
    size_t newSize = stride * cap;
    if (newSize <= 0) return arr;

    size_t* hdr = realloc((arr) ? _obeliskArrayHdr(arr) : NULL, newSize + hdrSize);
    if (!hdr) return NULL;

    hdr[0] = cap;
    if (arr == NULL) hdr[1] = 0;

    return hdr + OBELISK_ARRAY_HDR_SIZE;
}

void* _obeliskArrayReserve(void* arr, size_t stride, size_t newCap) {
    if (arr && newCap <= _obeliskArrayCap(arr)) return arr;

    // increase needed capacity to guarantee O(1) amortized
    if (newCap < 2 * obeliskArrayCap(arr))  newCap = 2 * obeliskArrayCap(arr);
    else if (newCap < 4)                    newCap = 4;

    return _obeliskArrayResize(arr, stride, newCap);
}

void* _obeliskInsertN(void* arr, size_t stride, size_t i, size_t n) {
    _obeliskArrayLen(arr) += n;
    return memmove((char*)arr + ((i + n) * stride), (char*)arr + (i * stride), (_obeliskArrayLen(arr) - (i + n)) * stride);
}

void* _obeliskRemoveN(void* arr, size_t stride, size_t i, size_t n) {
    _obeliskArrayLen(arr) -= n;
    return memmove((char*)arr + (i * stride), (char*)arr + ((i + n) * stride), (_obeliskArrayLen(arr) - i) * stride);
}