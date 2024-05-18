#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#define d_malloc(size) debug_malloc(size, __FILE__, __LINE__, __FUNCTION__)
#define d_free(ptr) debug_free(ptr, __FILE__, __LINE__, __FUNCTION__)
#define d_realloc(ptr, size) debug_realloc(ptr, size, __FILE__, __LINE__, __FUNCTION__)

typedef struct node {
  node *next;
  void *ptr;
  const char* file;
  int line; 
  const char *func;
} node;

node* head = NULL;

bool add_ptr_tracker(void* pointer, const char* file, int line, const char *func) {
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
  } else {
    node* tmp = head;
    while (tmp->next != NULL) {
      tmp = tmp->next;
    }
    node* new_node = (node*) malloc(sizeof(node));
    if (new_node == NULL) {
      return false;
    }
    new_node->next = NULL;
    new_node->ptr = pointer;
    new_node->file = file;
    new_node->func = func;
    new_node->line = line;
    tmp->next = new_node;
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
    while (tmp->ptr != pointer && tmp != NULL) {
      prev = tmp;
      tmp = tmp->next;
    }

    if (prev == NULL) {
      head = NULL;
    } else {
      prev->next = tmp->next;
      free(tmp);
    }
  }
}

void* debug_malloc(size_t size, const char* file, int line, const char *func) {
  void *pointer = malloc(size);
  printf("line=%d\tfunc=%s\tfile=%ssize=[%li]\n", line, func, file, size);
  if (!add_ptr_tracker(pointer)) {
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
  return new_ptr;
}

void detect_mem_leak() {
  node* tmp = head;
  printf("All current pointer:\n");
  while (tmp != NULL) {
    printf("\tline=%d\tfunc=%s\tfile=%ssize=[%li]\n", line, func, file, size);
    tmp = tmp->next;
  }
}