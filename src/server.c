#include "server.h"
#include "utils.h"

volatile int usr1_receive = 0;
volatile int chld_receive = 0;
// static pid_t childTerminated = 0;
static char **args = NULL;
static char clientFifo0Buffer[LENGTH_OF_CLIENT_FIFOS];
static char clientFifo1Buffer[LENGTH_OF_CLIENT_FIFOS];
static char clientFifo0BufferDel[LENGTH_OF_CLIENT_FIFOS];
static char clientFifo1BufferDel[LENGTH_OF_CLIENT_FIFOS];
struct list *pids = NULL;

void handSIGUSR1(int sig) {
	usr1_receive = 1;
}

void handENDSRV(int sig) {
	switch (sig) {
		case SIGINT: {
			fprintf(stderr, "Server terminated due to signal interrupt (SIGINT)\n");
			break;
		}
		case SIGQUIT: {
			fprintf(stderr, "Server terminated due to signal interrupt (SIGQUIT)\n");
			break;
		}
		case SIGTERM: {
			fprintf(stderr, "Server terminated due to signal interrupt (SIGTERM)\n");
			break;
		}
	}

	exit(2);
}

void handSIGCHLD(int sig) {
	printf("SIGCHLD recieved\n");

	int childEnding = 0;
	pid_t childTerminated = wait(&childEnding);
	interpret_end_of_game(childEnding, childTerminated);

	chld_receive = 1;
}

void initialization(void) {
	int fdPidServer = open(PATH_PID_FILE_SERVER, O_CREAT | O_EXCL | O_WRONLY, SERV_PID_MODE);

	if (fdPidServer == -1) {
		fprintf(stderr, "Error while opening %s\nThe server may be already running\n", PATH_PID_FILE_SERVER);
		perror("open");
		exit(1);
	}

	pid_t serverPid = getpid();
	char stringServerPid[SERVER_PID_SIZE];

	sprintf(stringServerPid, "%d", serverPid);

	ssize_t retOfWrite = write(fdPidServer, stringServerPid, strlen(stringServerPid) * sizeof(char));

	if (retOfWrite == -1) {
		fprintf(stderr, "Error while writing the server pid\n");
		perror("write");
		exit(1);
	}

	int resClose = close(fdPidServer);
	
	if (resClose == -1) {
		fprintf(stderr, "Error while closing %s\n", PATH_PID_FILE_SERVER);
		perror("close");
		exit(1);
	}

	struct stat statFifo;
    int statFifoExists = stat(PATH_FIFO_GAME_SERVER, &statFifo);

    if (statFifoExists == -1) {
    	int resMkfifo = mkfifo(PATH_FIFO_GAME_SERVER, FIFO_MODE);

    	if (resMkfifo == -1) {
    		fprintf(stderr, "Error while creating FIFO %s\n", PATH_FIFO_GAME_SERVER);
			perror("mkfifo");
			exit(1);
    	}
    }

    int resMkdir = mkdir(PATH_DIR_GAME_SERVER, DIR_MODE);

    if (resMkdir == -1) {
    	fprintf(stderr, "Error while creating directory %s\n", PATH_DIR_GAME_SERVER);
		perror("mkdir");
		exit(1);
    }
}

void define_signals_actions(void) {
	struct sigaction actionUsr1;
	sigemptyset(&actionUsr1.sa_mask);
	actionUsr1.sa_handler = handSIGUSR1;
	actionUsr1.sa_flags = 0;
	sigaction(SIGUSR1, &actionUsr1, NULL);

	struct sigaction actionChld;
	sigemptyset(&actionChld.sa_mask);
	actionChld.sa_handler = handSIGCHLD;
	actionChld.sa_flags = 0;
	sigaction(SIGCHLD, &actionChld, NULL);

	struct sigaction actionEndServ;
	sigemptyset(&actionEndServ.sa_mask);
	actionEndServ.sa_handler = handENDSRV;
	actionEndServ.sa_flags = 0;
	sigaction(SIGINT, &actionEndServ, NULL);
	sigaction(SIGQUIT, &actionEndServ, NULL);
	sigaction(SIGTERM, &actionEndServ, NULL);
}

void create_fifo_for_client(pid_t clientPID) {
	sprintf(clientFifo0Buffer, "%s%s%d%s", PATH_DIR_GAME_SERVER, "/cli", clientPID, "_0.fifo");

	int resMkfifo = mkfifo(clientFifo0Buffer, FIFO_MODE);

   	if (resMkfifo == -1) {
    	fprintf(stderr, "Error while creating FIFO %s\n", clientFifo0Buffer);
		perror("mkfifo");
		exit(1);
    }

    sprintf(clientFifo1Buffer, "%s%s%d%s", PATH_DIR_GAME_SERVER, "/cli", clientPID, "_1.fifo");

    resMkfifo = mkfifo(clientFifo1Buffer, FIFO_MODE);

   	if (resMkfifo == -1) {
    	fprintf(stderr, "Error while creating FIFO %s\n", clientFifo1Buffer);
		perror("mkfifo");
		exit(1);
    }
}

void send_sigusr1_to_client(pid_t clientPID) {
	int retOfKill = kill(clientPID, SIGUSR1);

	if (retOfKill == -1) {
		fprintf(stderr, "Server error : can't send signal SIGUSR1 to client\n");
		perror("kill");
	}
}

void send_sigusr2_to_client(pid_t clientPID) {
	int retOfKill = kill(clientPID, SIGUSR2);

	if (retOfKill == -1) {
		fprintf(stderr, "Server error : can't send signal SIGUSR2 to client\n");
		perror("kill");
	}
}

void free_tabs(void) {
	/*list_destroy(pids);

	free(pids);*/

	if (args != NULL) {
		size_t counter = 0;

		while (args[counter] != NULL) {
			free(args[counter]);
			counter++;
		}

		free(args);
	}

	fprintf(stdout, "MEMORY FREE COMPLETED SUCCESSFULLY\n");
}

void delete_childs(void) {
	/*struct list_node *node = pids -> first;

    while (node != NULL){
    	if (node->isAlive) {
    		send_sigusr2_to_client(node->clientPID);
    	}
        node = node -> next;
    }*/
}

void remove_fifos(pid_t clientPID) {
	sprintf(clientFifo0BufferDel, "%s%s%d%s", PATH_DIR_GAME_SERVER, "/cli", clientPID, "_0.fifo");

	int resUnlink = unlink(clientFifo0BufferDel);

	if (resUnlink == -1 && errno != ENOENT) {
		fprintf(stderr, "Error while removing FIFO %s\n", clientFifo0BufferDel);
		perror("unlink");
	}

	sprintf(clientFifo1BufferDel, "%s%s%d%s", PATH_DIR_GAME_SERVER, "/cli", clientPID, "_1.fifo");

	resUnlink = unlink(clientFifo1BufferDel);

	if (resUnlink == -1 && errno != ENOENT) {
		fprintf(stderr, "Error while removing FIFO %s\n", clientFifo1BufferDel);
		perror("unlink");
	}
}

void deinitialization(void) {
	int resUnlink = unlink(PATH_PID_FILE_SERVER);

	if (resUnlink == -1 && errno != ENOENT) {
		fprintf(stderr, "Error while removing %s\n", PATH_PID_FILE_SERVER);
		perror("unlink");
	}

	resUnlink = unlink(PATH_FIFO_GAME_SERVER);

	if (resUnlink == -1 && errno != ENOENT) {
		fprintf(stderr, "Error while removing FIFO %s\n", PATH_FIFO_GAME_SERVER);
		perror("unlink");
	}

	DIR *dir = opendir(PATH_DIR_GAME_SERVER);

    if (dir != NULL) {
		struct dirent *info = readdir(dir);

		while (info != NULL) {
			if (info->d_name == NULL) {
				break;
			}

			if (info->d_name[0] != '.') {
				size_t sizeOfNameFifo = LENGTH_OF_PATH_DIR + strlen(info->d_name) + 2;
				char *nameOfFifo = calloc(sizeOfNameFifo, sizeof(char));
				sprintf(nameOfFifo, "%s%c%s", PATH_DIR_GAME_SERVER, '/', info->d_name);

				resUnlink = unlink(nameOfFifo);

				if (resUnlink == -1 && errno != ENOENT) {
					fprintf(stderr, "Error while removing FIFO %s\n", info->d_name);
					perror("unlink");
					free(nameOfFifo);
					exit(1);
				}

				free(nameOfFifo);
			}

			info = readdir(dir);
		}

		int retOfCosedir = closedir(dir);

		if (retOfCosedir == -1) {
			fprintf(stderr, "Error while closing directory %s\n", PATH_DIR_GAME_SERVER);
			perror("closedir");
		}
	}

	int resOfRmdir = rmdir(PATH_DIR_GAME_SERVER);

	if (resOfRmdir == -1 && errno != ENOENT) {
		fprintf(stderr, "Error while removing directory %s\n", PATH_DIR_GAME_SERVER);
		perror("rmdir");
	}

	fprintf(stdout, "FILES SUCCESSFULLY REMOVED\n");
}

void get_client_informations(pid_t *clientPID) {
	int fdFifo = open(PATH_FIFO_GAME_SERVER, O_RDONLY);

	if (fdFifo == -1) {
		fprintf(stderr, "Error while opening %s\n", PATH_FIFO_GAME_SERVER);
		perror("open");
		exit(1);
	}

	char *clientPIDBuffer = recv_string(fdFifo);

	if (clientPIDBuffer == NULL) {
		fprintf(stderr, "Error while getting the client informations\n");
		free(clientPIDBuffer);
		return;
	}

	*clientPID = atoi(clientPIDBuffer);
	free(clientPIDBuffer);

	if (args != NULL) {
		free_tabs();
	}

	args = recv_argv(fdFifo);

	if (args == NULL) {
		fprintf(stderr, "Error while getting the client informations\n");
	}

	int resClose = close(fdFifo);
	
	if (resClose == -1) {
		fprintf(stderr, "Error while closing %s\n", PATH_FIFO_GAME_SERVER);
		perror("close");
		exit(1);
	}
}

bool game_exists(char *game) {
	return access(game, X_OK) != -1;
}

pid_t start_game() {
	pid_t pid = fork();

	switch (pid) {
		case -1: {
			fprintf(stderr, "Error while creating child to start game\n");
			perror("fork");
			exit(1);
		}
		case 0: {
			struct sigaction actionIgn;
			sigemptyset(&actionIgn.sa_mask);
			actionIgn.sa_handler = SIG_IGN;
			actionIgn.sa_flags = 0;
			sigaction(SIGINT, &actionIgn, NULL);
			sigaction(SIGQUIT, &actionIgn, NULL);
			sigaction(SIGTERM, &actionIgn, NULL);
			sigaction(SIGUSR1, &actionIgn, NULL);

			int fd0 = open(clientFifo0Buffer, O_RDONLY);

	        if (fd0 == -1) {
	            fprintf(stderr, "Error while opening FIFO %s\n", clientFifo0Buffer);
	            perror("open");
	            _exit(1);
	        }

	        int fd1 = open(clientFifo1Buffer, O_WRONLY);

	        if (fd1 == -1) {
	            fprintf(stderr, "Error while opening FIFO %s\n", clientFifo1Buffer);
	            perror("open");
	            _exit(1);
	        }

	        int retOfDup2 = dup2(fd0, 0);

	        if (retOfDup2 == -1) {
	        	perror("dup2");
	        	_exit(1);
	        }

	        retOfDup2 = dup2(fd1, 1);

	        if (retOfDup2 == -1) {
	        	perror("dup2");
	        	_exit(1);
	        }

	        int retOfClose = close(fd0);

	        if (retOfClose == -1) {
	        	perror("close");
	        	_exit(1);
	        }

	        retOfClose = close(fd1);

	        if (retOfClose == -1) {
	        	perror("close");
	        	_exit(1);
	        }

			execv(args[0], args);

			perror("execv");
			_exit(3);
		}
		default: {
			return pid;
		}
	}
}

void interpret_end_of_game(const int childEnding, const pid_t childPID) {
	if (WIFEXITED(childEnding)) {
		printf("Game %d exited normally, status=%d\n", childPID, WEXITSTATUS(childEnding));
	} else if (WIFSIGNALED(childEnding)) {
		printf("Game %d killed by signal %d\n", childPID, WTERMSIG(childEnding));
	}
}

bool edit_table() {
	if (args == NULL || args[0] == NULL) {
		return true;
	}

	size_t lengthOfGame = strlen(args[0]) + LENGTH_OF_SERV_EXT + 1;

	char *gameName = calloc(lengthOfGame, sizeof(char));
	sprintf(gameName, "%s%s", args[0], SERV_EXT);

	free(args[0]);
	args[0] = gameName;

	return false;
}

int main(int argc, char **argv) {
	initialization();
	atexit(deinitialization);
	atexit(free_tabs);
	atexit(delete_childs);

	define_signals_actions();

	bool argsError = false;

	pid_t childPID = 0;

	/*pids = malloc(sizeof(struct list));

	list_create(pids);

	struct list_node *newNode = NULL;*/
	
	while(1){
		pause();

		if (usr1_receive) {
			pid_t clientPID = 0;

			get_client_informations(&clientPID);

			argsError = edit_table();

			if (!argsError && game_exists(args[0])) {
				create_fifo_for_client(clientPID);
				send_sigusr1_to_client(clientPID);
				childPID = start_game();

				#ifdef DEBUG
				printf("CHILD PID : %d\n", childPID);
				#endif
				
				/*newNode = malloc(sizeof(struct list_node));
				newNode->clientPID = clientPID;
				newNode->serverPID = childPID;
				newNode->isAlive = true;
				list_push_back(pids, newNode);*/
			} else {
				send_sigusr2_to_client(clientPID);
			}

			usr1_receive = 0;
		} else if (chld_receive) {
			/*pid_t cliPID = 0;
			size_t index = list_search_server(pids, childTerminated, &cliPID);
			
			if (cliPID != 0) {
				send_sigusr2_to_client(cliPID);
			}

			list_remove(pids, index);

			remove_fifos(cliPID);*/

			chld_receive = 0;
		}
	}

	return 0;
}
