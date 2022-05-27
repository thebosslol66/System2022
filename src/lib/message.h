#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#ifndef H_LIB_MESSAGE
#define H_LIB_MESSAGE

/* Write a string in the file descriptor */
int send_string(int fd, char *str);
/* Return a string read from the file descriptor */
char *recv_string(int fd);
/* Send a list of string terminated by NULL to the file descriptor */
int send_argv(int fd, char *argv[]);
/* Recieve a list of string terminated by NULL from file descriptor */
char **recv_argv(int fd);
/* Free a list of string received by recv_argv */
void free_recv_argv(char ** mes);

#endif