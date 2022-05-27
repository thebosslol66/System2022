#ifndef H_UTILS
#define H_UTILS

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

/* Get a random number on 32 bits */
unsigned int randInt(unsigned int max);
/* Return if a string contains a space */
int have_space(char * str);
/* flush a file descriptor with extra chars */
void flush(FILE * fp);
/* count line number in a file */
size_t count_file_lines(FILE * fp);
/* Get the digits nunmber in a ginven number */
int digit_in_number(int nb);


/* A structure to link server and client */
struct list_node {
    pid_t clientPID;
    pid_t serverPID;
    bool isAlive;
    struct list_node *next;
};

struct list {
    struct list_node *first;
};

/* Create a linked list */
void list_create(struct list *self);
/* Destroy a linked list */
void list_destroy(struct list *self);
/* Get if the list is empty */
bool list_empty(const struct list *self);
/* Append a new node at the end of list */
void list_push_back(struct list *self, struct list_node *newNode);
/* Remove safely a node in the list at the index given */
void list_remove(struct list *self, size_t index);
/* Set isAlive to false for the serverPID given */
bool list_kill_client(struct list *self, pid_t serverPID);
/* Get the index of the node with the serverPID and set to the clientPID the client linked to */
size_t list_search_server(struct list *self, pid_t serverPID, pid_t *clientPID);

#endif //H_UTILS