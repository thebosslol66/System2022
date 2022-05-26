#ifndef H_UTILS
#define H_UTILS

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct list_node {
    pid_t clientPID;
    pid_t serverPID;
    bool isAlive;
    struct list_node *next;
};

struct list {
    struct list_node *first;
};

void list_create(struct list *self);
void list_destroy(struct list *self);
bool list_empty(const struct list *self);
void list_push_back(struct list *self, struct list_node *newNode);
void list_remove(struct list *self, size_t index);

unsigned int randInt(unsigned int max);
int have_space(char * str);
void flush(FILE * fp);
size_t count_file_lines(FILE * fp);

#endif //H_UTILS