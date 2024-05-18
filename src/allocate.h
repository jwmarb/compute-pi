#ifndef ALLOCATE_H

#define d_malloc(size) debug_malloc(size, __FILE__, __LINE__, __FUNCTION__)
#define d_free(ptr) debug_free(ptr, __FILE__, __LINE__, __FUNCTION__)
#define d_realloc(ptr, size) debug_realloc(ptr, size, __FILE__, __LINE__, __FUNCTION__)

void* debug_malloc(size_t, const char*, int, const char*);
void debug_free(void*, const char*, int, const char*);
void* debug_realloc(void*, size_t, const char*, int, const char*);

void detect_mem_leak();

#endif