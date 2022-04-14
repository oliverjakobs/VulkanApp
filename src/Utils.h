#ifndef OBELISK_UTILS
#define OBELISK_UTILS

#include "common.h"

#define OBELISK_ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

char* obeliskReadFile(const char* path, size_t* sizeptr);

#endif // !OBELISK_UTILS
