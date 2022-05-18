
#include "defs.h"

int read_server_pid(){
    if(access(PID_AD_FILE, F_OK) == -1 || access(PID_AD_FILE, R_OK) == -1){
        fprintf(stderr, "Server is not running\n");
        exit(3);
    }
    int fd = open(PID_AD_FILE, O_RDONLY);
    if (fd == -1){
        dperror("open");
        exit(3);
    }
    int servPID;
    if (read(fd, &servPID, sizeof(int)) == -1){
        dperror("read");
        exit(3);
    }
    return servPID;
}

int digit_in_number(int nb){
    int i = 0;
    do
    {
        nb = nb/10;
        i++;
    } while(nb > 0);
    return i;
}


/* handler du signal SIGUSR1 */
void handSIGUSR1(int sig) {
}

char *progCliName;
/* handler du signal SIGUSR2 */
void handSIGUSR2(int sig) {
    free(progCliName);
    exit(4);
}

int main(int argc, char *argv[]){
    if (argc < 2){
        //erreur?
        fprintf(stderr, "Usage: %s <game> [args]...\n", argv[0]);
        exit(1);
    }
    size_t sz = strlen(argv[1])+4;
    progCliName = calloc(sz+1, sizeof(char));
    sprintf(progCliName, "%s_cli", argv[1]);
    if (access(progCliName, F_OK) != 0 || access(progCliName, X_OK) != 0){
        fprintf(stderr, "%s doesn't exist or isn't runnable\n", progCliName);
        exit(2);
    }

    struct sigaction action1, action2;
    action1.sa_flags = 0;
    sigemptyset(&action1.sa_mask);
    action1.sa_handler = handSIGUSR1;
    action2.sa_flags = 0;
    sigemptyset(&action2.sa_mask);
    action2.sa_handler = handSIGUSR2;
    sigaction(SIGUSR1, &action1, NULL);
    sigaction(SIGUSR2, &action2, NULL);

    sigset_t new, old, suspend;
    sigemptyset(&new);
    sigaddset(&new, SIGUSR1);
    sigprocmask(SIG_BLOCK, &new, &old);

    int servPID = read_server_pid();



    kill(servPID, SIGUSR1);
    int fd = open(SERVER_FIFO, O_WRONLY);
    if (fd == -1){
        dperror("open");
        exit(5);
    }
    int pid = getpid();
    if (write(fd, &pid, sizeof(int)) == -1){
        dperror("write");
        exit(5);
    }
    if (send_argv(fd, argv+1) == -1) {
        dperror("send_argv");
        exit(5);
    }

    suspend = old;
    sigdelset(&suspend, SIGUSR1);
    sigdelset(&suspend, SIGUSR2);
    sigsuspend(&suspend);

    sigprocmask(SIG_SETMASK, &old, NULL);
    

    size_t pathLength = strlen(FIFO_DIR)+4+digit_in_number(pid)+8;
    char * to_serv = calloc(pathLength+1, sizeof(char));
    char * from_serv = calloc(pathLength+1, sizeof(char));
    sprintf(to_serv, "%s/cli%d_0.fifo", FIFO_DIR, pid);
    sprintf(from_serv, "%s/cli%d_1.fifo", FIFO_DIR, pid);


    int rfd = open(from_serv, O_RDONLY);
    int wfd = open(to_serv, O_WRONLY);

    if (rfd == -1 || wfd == -1){
        dperror("open");
        exit(8);
    }

    dup2(wfd, 3);
    dup2(rfd, 4);
    
    #ifdef VALGRIND
    free(progCliName);
    progCliName = calloc(sz+3, sizeof(char));
    sprintf(progCliName, "./%s_cli", argv[1]);
    char **nargv = calloc(argc+4, sizeof(char*));
    nargv[0] = "valgrind";
    nargv[1] = "-s";
    nargv[2] = "--leak-check=full";
    nargv[3] = progCliName;
    for (int i=1; i<argc; i++){
        nargv[i+3] = argv[i];
    }
    execvp(nargv[0], nargv);
    #else
    argv[1] = progCliName;
    execv(progCliName, argv+1);
    #endif
    dperror("execv");
    return 6;
}