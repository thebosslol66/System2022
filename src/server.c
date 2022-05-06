#include "defs.h"

#define LOG_FILE "/tmp/game_server.err"

volatile int usr1_receive = 0;

/* handler du signal SIGUSR1 */
void handSIGUSR1(int sig) {
    usr1_receive = 1;
}

int currentlyExecute = 1;
int childNb = 0;
void handKILLSERV(int sig){
    currentlyExecute = 0;
}

void handSIGCHLD(int sig){
    int status;
    childNb--;
    wait(&status);
    if (WIFEXITED(status)){
        fprintf(stderr, "Processus fils terminé normalement: %d\n", WEXITSTATUS(status));
    }
    if (WIFSIGNALED(status)){
        fprintf(stderr, "Processus fils terminé anormalement: %d\n", WTERMSIG(status));
    }
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

int main(int argc, char *argv[]){
    close(STDERR_FILENO);
	if (!isatty(STDERR_FILENO) && errno == EBADF){
		// int output = open(LOG_FILE, O_CREAT | O_APPEND, S_IRUSR|S_IWUSR);
        // if (output < 0){
        //     fprintf(stderr, "Unable to open log file.\n");
        //     perror("open");
		// 	exit(EXIT_FAILURE);
        // }
        // lseek(output, 0, SEEK_END);
		// if(dup2(output, STDERR_FILENO) < 0) {
		// 	fprintf(stderr,"Unable to duplicate file descriptor.\n");
        //     perror("dup2");
		// 	exit(EXIT_FAILURE);
		// }
        // fprintf(stdout, "It works\n");
        freopen(LOG_FILE, "a+", stderr);
	}

	fprintf(stderr, "It works\n");

    if(access(PID_AD_FILE, F_OK) == 0){
        #ifdef DEBUG
        fprintf(stderr, "An other instance of the server is running\n");
        #endif
        exit(1);
    }

    int fd = open(PID_AD_FILE, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        dperror("open");
        exit(2);
    }
    int pid = getpid();
    ssize_t sz = write(fd, &pid, sizeof(int));
    close(fd);
    fd=-1;

    if (sz < 1) {
        dperror("write");
        exit(3);
    }

    if(access(SERVER_FIFO, F_OK) == -1){
        if (mkfifo(SERVER_FIFO, 0666) == -1){
            dperror("mkfifo");
            exit(4);
        }
    }

    struct stat sb;
    if (!(stat(FIFO_DIR, &sb) == 0 && S_ISDIR(sb.st_mode))) {
        if (mkdir(FIFO_DIR, 0755) == -1){
            dperror("mkdir");
            exit(5);
        }
    }

    struct sigaction action;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    action.sa_handler = handSIGUSR1;
    sigaction(SIGUSR1, &action, NULL);

    struct sigaction action2;
    action2.sa_flags = 0;
    sigemptyset(&action2.sa_mask);
    action2.sa_handler = handKILLSERV;
    sigaction(SIGINT, &action2, NULL);
    sigaction(SIGQUIT, &action2, NULL);
    sigaction(SIGTERM, &action2, NULL);


    struct sigaction actionCHLD;
    actionCHLD.sa_flags = 0;
    sigemptyset(&actionCHLD.sa_mask);
    actionCHLD.sa_handler = handSIGCHLD;
    sigaction(SIGCHLD, &actionCHLD, NULL);


    while(currentlyExecute){

        pause();

        if (usr1_receive){
            int fd = open(SERVER_FIFO, O_RDONLY);
            int clientPID;
            ssize_t sz = read(fd, &clientPID, sizeof(int));
            int haveError = 0;
            if (sz < 1){
                dperror("read");
                close(fd);
            }
            else{
                char **programParam = recv_argv(fd);
                if (programParam == NULL){
                    dperror("recv_argv");
                    haveError = 1;
                    close(fd);
                }
                else{
                    close(fd);
                    size_t sz = strlen(programParam[0])+5;
                    char *temp = calloc(sz+1, sizeof(char));
                    sprintf(temp, "%s_serv", programParam[0]);
                    free(programParam[0]);
                    programParam[0] = temp;
                    if (access(programParam[0], F_OK) != 0 || access(programParam[0], X_OK) != 0){
                        #ifdef DEBUG
                        fprintf(stderr, "%s doesn't exist or isn't runnable\n", programParam[0]);
                        #endif
                        haveError = 1;
                    }
                    else{
                        size_t pathLength = strlen(FIFO_DIR)+4+digit_in_number(clientPID)+8;
                        char * from_cli = calloc(pathLength+1, sizeof(char));
                        char * to_cli = calloc(pathLength+1, sizeof(char));
                        sprintf(from_cli, "%s/cli%d_0.fifo", FIFO_DIR, clientPID);
                        sprintf(to_cli, "%s/cli%d_1.fifo", FIFO_DIR, clientPID);
                        if (mkfifo(from_cli, 0666) == -1){
                            dperror("mkfifo");
                            haveError = 1;
                        }
                        if (mkfifo(to_cli, 0666) == -1){
                            dperror("mkfifo");
                            haveError = 1;
                        }

                        if (haveError == 0){

                            kill(clientPID, SIGUSR1);

                            int wfd = open(to_cli, O_WRONLY);
                            int rfd = open(from_cli, O_RDONLY);
							free(from_cli);
							free(to_cli);

                            if (rfd == -1 || wfd == -1){
                                dperror("open");
                                haveError = 1;
                            }
                            else {
                                if (currentlyExecute != 0){
                                    switch(fork()){
                                        case -1:{
                                            dperror("fork");
                                            haveError = 1;
                                        }
                                        case 0:{
                                            dup2(rfd, STDIN_FILENO);
                                            dup2(wfd, STDOUT_FILENO);
                                            #ifdef VALGRIND
                                            int len = 0;
                                            while (programParam[len] != NULL){
                                                len++;
                                            }
                                            char * progServName = calloc(strlen(programParam[0])+3, sizeof(char));
                                            sprintf(progServName, "./%s", programParam[0]);
                                            char **nargv = calloc(len+4, sizeof(char*));
                                            nargv[0] = "valgrind";
                                            nargv[1] = "-s";
                                            nargv[2] = "--leak-check=full";
                                            nargv[3] = progServName;
                                            for (int i=1; i<len; i++){
                                                nargv[i+3] = programParam[i];
                                            }
                                            execvp(nargv[0], nargv);
											free(progServName);
											free(nargv);
                                            #else
                                            execv(programParam[0], programParam);
											
                                            #endif
											close(wfd);
											close(rfd);
											free_recv_argv(programParam);
                                            dperror("execv");
											exit(1);
                                        }
                                        default:
                                            break;
                                    }
                                    childNb++;
                                }
                                else {
                                    haveError = 1;
                                }
                                
                            }
							close(wfd);
							close(rfd);
                        }
                    }
                }
                if (haveError != 0){
                    kill(clientPID, SIGUSR2);
                }
				free_recv_argv(programParam);
            }
            usr1_receive = 0;
        }
    }

    for (int i=0; i< childNb; i++){
        wait(NULL);
    }
	unlink(PID_AD_FILE);
    fflush(stderr);
    fclose(stdout);
    
    return 0;
}