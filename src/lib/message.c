#include "message.h"

int send_string(int fd, char *str){
    int stringLenght = strlen(str);
    ssize_t sz = write(fd, &stringLenght, sizeof(int));
    if (sz == -1){
        return -1;
    }
    sz = write(fd, str, sizeof(char)*stringLenght);
    if (sz == -1){
        return -1;
    }
    #ifdef DEBUG
    fprintf(stderr, "\tSending string(%d): %s\n", stringLenght, str);
    #endif
    return sz;
}

char *recv_string(int fd){
    int stringLenght;
    ssize_t sz = read(fd, &stringLenght, sizeof(int));
    if (sz < 1){
        return NULL;
    }
    char *str = calloc(stringLenght+1, sizeof(char));
    sz = read(fd, str, sizeof(char)*stringLenght);
    if (sz < 1){
        free(str);
        return NULL;
    }
    #ifdef DEBUG
    fprintf(stderr, "\tRecieve string(%d): %s\n", stringLenght, str);
    #endif
    return str;
}

int send_argv(int fd, char *argv[]){
    int tabLength=0;
    int pos=0;
    while(argv[pos] != NULL){
        pos++;
        tabLength++;
    }
    #ifdef DEBUG
    fprintf(stderr, "Send: %d words\n", tabLength);
    #endif
    ssize_t sz = write(fd, &tabLength, sizeof(int));
    if (sz == -1){
        return -1;
    }
    pos=0;
    while(argv[pos] != NULL){
        sz = send_string(fd, argv[pos]);
        if (sz == -1){
            return -1;
        }
        pos++;
    }

    return sz;

}

char **recv_argv(int fd){
    int tabLength;
    ssize_t sz = read(fd, &tabLength, sizeof(int));
    if (sz == -1){
        return NULL;
    }
    #ifdef DEBUG
    fprintf(stderr, "Wait: %d words\n", tabLength);
    #endif
    char **argv = calloc(tabLength+1, sizeof(char*));
    for (int i = 0; i <tabLength; i++){
        argv[i] = recv_string(fd);
        if (argv[i] == NULL){
            return NULL;
        }
    }
    return argv;
}

void free_recv_argv(char ** mes){
    int i=0;
    while (mes[i] != NULL){
        free(mes[i]);
        i++;
    }
    free(mes);
}