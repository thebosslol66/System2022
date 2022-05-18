#include "launch_daemon.h"

int main (int argc, char * argv[]){
    if (argc != 2){
        printf("Usage: %s program\n", argv[0]);
        return 1;
    }

    if (access(argv[1], F_OK) != 0 || access(argv[1], X_OK) != 0){
        fprintf(stderr, "%s doesn't exist or isn't runnable\n", argv[1]);
        exit(2);
    }
    pid_t child = fork();
    if (child == -1){
        perror("fork");
        exit(3);
    }
    if (child == 0 ){
        umask(0);
        pid_t proc = setsid();
        if (proc == -1){
            perror("setsid");
            exit(4);
        }
        child = fork();
        if (child == -1){
            perror("fork");
            exit(3);
        }

        if (child == 0){
            #ifdef DEBUG
                printf("Daemon PID: %d\n", getpid());
            #endif
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            #ifdef VALGRIND
                int len = argc-1;
                char * progServName = calloc(strlen(argv[1])+3, sizeof(char));
                sprintf(progServName, "./%s", argv[1]);
                char **nargv = calloc(len+4, sizeof(char*));
                nargv[0] = "valgrind";
                nargv[1] = "-s";
                nargv[2] = "--leak-check=full";
                nargv[3] = progServName;
                for (int i=1; i<len; i++){
                    nargv[i+3] = argv[i];
                }
                execvp(nargv[0], nargv);
                free(progServName);
            #else
                execv(argv[1], argv+1);
            #endif
            perror("execv");
            exit(6);
        }
        exit(0);
    }
    int statu;
    wait(&statu);
    return statu;
}