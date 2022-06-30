#define _MM_H 1
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

// TODO include header file
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
// TODO define structure for memory informaiton
struct MemAlloc {
    void* ptr;
    void* payload;
    size_t size;
    size_t allocSize;
    struct MemAlloc *prev;
    struct MemAlloc *next;
};
// TODO Function declaration
void *mm_malloc(size_t size);
void *mm_calloc(size_t num, size_t size);
void *mm_realloc(void *ptr, size_t size);
void mm_free(void *ptr);
void mm_status();


#define ALIGNMENT 32
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define CHUNK_SIZE (1 << 14)
#define CHUNK_ALIGN(size) (((size)+(CHUNK_SIZE-1)) & ~(CHUNK_SIZE-1))

size_t current_avail_size = 0;
void* current_avail;
struct MemAlloc head = {NULL, NULL, 0, 0, NULL, NULL};
struct MemAlloc tail = {NULL, NULL, 0, 0, NULL, NULL};
int isInit = 0;
void mm_status()
{
    if (!isInit) {
        printf("Memory manager is not initialized\n");
        return;
    }
    printf("Allocated Memory:\n");
    int idx = 0;
    struct MemAlloc* curr = tail.prev;
    while (curr != &head) {
        printf("[%d]: Addr=%p, Size =%lu\n", idx++, curr->payload, curr->size);
        curr = curr->prev;
    }
}

void mm_init()
{
    //printf("mm_init\n");
    head.next = &tail;
    head.prev = &head;
    tail.next = &tail;
    tail.prev = &head;
}

void mm_insert_head(struct MemAlloc* memAlloc)
{
   // printf("mm_insert_head\n");
    memAlloc->prev = &head;
    memAlloc->next = head.next;
    head.next->prev = memAlloc;
    head.next = memAlloc;
}

void mm_insert_tail(struct MemAlloc* memAlloc)
{
    memAlloc->prev = tail.prev;
    memAlloc->next = &tail;
    tail.next->prev = memAlloc;
    tail.prev = memAlloc;
}

void mm_remove(struct MemAlloc* memAlloc)
{
    memAlloc->prev->next = memAlloc->next;
    memAlloc->next->prev = memAlloc->prev;
}

void *mm_malloc(size_t size)
{
    if (!isInit) {
        mm_init();
        isInit = 1;
    }
    size_t allocSize = ALIGN(size + sizeof(struct MemAlloc));
    void *p;
    //printf("mmap\n");
    p = mmap(0, allocSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    struct MemAlloc* memAlloc = (struct MemAlloc*)p;
    memAlloc->ptr = p;
    memAlloc->allocSize = allocSize;
    memAlloc->payload = p + sizeof(struct MemAlloc);
    memAlloc->size = size;
    mm_insert_head(memAlloc);
    return memAlloc->payload;
}
void *mm_calloc(size_t num, size_t size)
{
    if (!isInit) {
        mm_init();
        isInit = 1;
    }
    size_t allocSize = ALIGN(num * size + sizeof(struct MemAlloc));
    void *p;
    p = mmap(0, allocSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    struct MemAlloc* memAlloc = (struct MemAlloc*)p;
    memAlloc->ptr = p;
    memAlloc->allocSize = allocSize;
    memAlloc->payload = p + sizeof(struct MemAlloc);
    memAlloc->size = num * size;
    mm_insert_head(memAlloc);
    return memAlloc->payload;
}

void *mm_realloc(void *ptr, size_t size)
{
    if (!isInit) {
        mm_init();
        isInit = 1;
    }
    struct MemAlloc* iter = NULL;
    for (iter = &head; iter != &tail; iter = iter->next) {
        if (iter->payload == ptr) {
            if (iter->size >= size) {
                return ptr;
            }
            else {
                size_t allocSize = ALIGN(size + sizeof(struct MemAlloc));
                void*p = mremap(iter->ptr, iter->allocSize, allocSize, MREMAP_MAYMOVE);
                iter->ptr = p;
                iter->allocSize = allocSize;
                iter->payload = p + sizeof(struct MemAlloc);
                iter->size = size;
                return p;
            }
        }
    }
}

void mm_free(void *ptr)
{
    struct MemAlloc* iter;
    for (iter = &head; iter != &tail; iter = iter->next) {
        if (iter->payload == ptr) {
            mm_remove(iter);
            munmap(iter->ptr, iter->allocSize);
            return;
        }
    }
}