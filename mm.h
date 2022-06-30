#define _MM_H 1

#include <stdio.h>
#include <stdlib.h>

// TODO include header files 

// TODO define structure for memory informaiton  

// TODO Function declaration  
void *mm_malloc(size_t size);
void *mm_calloc(size_t num, size_t size);
void *mm_realloc(void *ptr, size_t size);
void mm_free(void *ptr);
void mm_status();
