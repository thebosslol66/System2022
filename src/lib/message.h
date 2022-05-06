#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#ifndef H_LIB_MESSAGE
#define H_LIB_MESSAGE

int send_string(int fd, char *str);
char *recv_string(int fd);
int send_argv(int fd, char *argv[]);
char **recv_argv(int fd);
void free_recv_argv(char ** mes);

#endif