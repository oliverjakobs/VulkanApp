#include "ignis_core.h"

#include <stdlib.h>

void* ignisAlloc(size_t size) { return malloc(size); }
void  ignisFree(void* block, size_t size)  { free(block); }