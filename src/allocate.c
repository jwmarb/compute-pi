#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#define d_malloc(size) debug_malloc(size, __FILE__, __LINE__, __FUNCTION__)
#define d_free(ptr) debug_free(ptr, __FILE__, __LINE__, __FUNCTION__)
#define d_realloc(ptr, size) debug_realloc(ptr, size, __FILE__, __LINE__, __FUNCTION__)

typedef struct node {
  struct node* next;
  void *ptr;
  const char* file;
  int line; 
  const char *func;
  size_t size;
} node;

node* head = NULL;

bool add_ptr_tracker(void* pointer, size_t size, const char* file, int line, const char *func) {
  if (head == NULL) {
    head = (node*) malloc(sizeof(node));
    if (head == NULL) {
      return false;
    }
    head->next = NULL;
    head->ptr = pointer;
    head->file = file;
    head->line = line;
    head->func = func;
    head->size = size;
  } else {
    node* new_node = (node*) malloc(sizeof(node));
    if (new_node == NULL) {
      return false;
    }
    new_node->next = head;
    new_node->ptr = pointer;
    new_node->file = file;
    new_node->func = func;
    new_node->line = line;
    new_node->size = size;
    head = new_node;
  }

  return true;
}

void remove_ptr_tracker(void* pointer, const char* file, int line, const char *func) {
  if (head == NULL) {
    fprintf(stderr, "Tried to free an invalid pointer:\n");
    fprintf(stderr, "\tLine: %d\n", line);
    fprintf(stderr, "\tFunction: %s\n", func);
    fprintf(stderr, "\tFile: %s\n", file);
  } else {
    node* tmp = head, *prev = NULL;
    while (tmp != NULL && tmp->ptr != pointer) {
      prev = tmp;
      tmp = tmp->next;
    }

    printf("[free]: line=%d\tfunc=%s\tfile=%s\tsize=[%li]\n", tmp->line, tmp->func, tmp->file, tmp->size);

    if (prev == NULL) {
      free(head);
      head = NULL;
    } else {
      prev->next = tmp->next;
      free(tmp);
    }
  }
}

void* debug_malloc(size_t size, const char* file, int line, const char *func) {
  void *pointer = malloc(size);
  printf("[malloc]: line=%d\tfunc=%s\tfile=%s\tsize=[%li]\n", line, func, file, size);
  if (!add_ptr_tracker(pointer, size, file, line, func)) {
    fprintf(stderr, "malloc() failed!\n");
    exit(EXIT_FAILURE);
  }
  return pointer;
}

void debug_free(void* pointer, const char* file, int line, const char *func) {
  free(pointer);
  remove_ptr_tracker(pointer, file, line, func);
}

void* debug_realloc(void* pointer, size_t size, const char* file, int line, const char *func) {
  void* new_ptr = realloc(pointer, size);
  node* tmp = head;
  while (tmp != NULL && tmp->ptr != pointer) {
    tmp = tmp->next;
  }

  tmp->ptr = new_ptr;
  tmp->size = size;

  printf("[realloc]: line=%d\tfunc=%s\tfile=%s\tsize=[%li]\n", tmp->line, tmp->func, tmp->file, tmp->size);

  return new_ptr;
}

void detect_mem_leak() {
  node* tmp = head;
  printf("All current pointer(s):\n");
  while (tmp != NULL) {
    printf("\tline=%d\tfunc=%s\tfile=%s\tsize=[%li]\n", tmp->line, tmp->func, tmp->file, tmp->size);
    tmp = tmp->next;
  }
}